#include <WiFi.h>
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define MAX_SPAM_APS 50

extern uint8_t beacon_template[];

struct SpamAP {
  uint8_t ssid[33];
  uint8_t ssid_len;
  uint8_t bssid[6];
};

static SpamAP spamPool[MAX_SPAM_APS];

static const char *base_names[] = { "TP-Link", "Linksys", "Netgear", "ASUS", "D-Link", "Home", "Office", "Starlink", "EastWest" };
static const char *suffixes[] = { "_WiFi", "-Guest", "-5G", "_Secure", "" };
static const char *emojis[] = { "🔥","📶","🚀","✨","⚡" };

static const char *rick_lyrics[] = {
  "Never Gonna Give You Up", "Never Gonna Let You Down",
  "Never Gonna Run Around", "And Desert You",
  "Never Gonna Make You Cry", "Never Gonna Say Goodbye",
  "Never Gonna Tell A Lie", "And Hurt You"
};

static const char *troll_names[] = {
  "FBI Surveillance Van 04", "Virus.exe", "Get Off My LAN",
  "Free Public WiFi", "Loading...", "Searching...", "Click for virus"
};

void generateSpamAPs(uint8_t mode, uint8_t count) {
  bsAPCount = (count > 0 && count <= MAX_SPAM_APS) ? count : 20;

  for (int i = 0; i < bsAPCount; i++) {
    char final[33] = {0};

    switch(mode) {
      case 0: { // COMMON
        int b = random(0, sizeof(base_names)/sizeof(base_names[0]));
        int s = random(0, sizeof(suffixes)/sizeof(suffixes[0]));
        snprintf(final, sizeof(final), "%s%s", base_names[b], suffixes[s]);
        if (random(0, 100) < 10) {
          strncat(final, emojis[random(0, 5)], sizeof(final)-strlen(final)-1);
        }
        break;
      }
      case 1: { // GARBAGE
        const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;";
        int len = 8 + random(0, 12);
        for (int j = 0; j < len; j++) final[j] = charset[random(0, sizeof(charset)-1)];
        break;
      }
      case 2: { // RICK ROLL
        strncpy(final, rick_lyrics[i % (sizeof(rick_lyrics)/sizeof(rick_lyrics[0]))], 32);
        break;
      }
      case 3: { // SECURITY
        strncpy(final, troll_names[i % (sizeof(troll_names)/sizeof(troll_names[0]))], 32);
        break;
      }
    }

    size_t len = strlen(final);
    if (len > 32) len = 32;
    memcpy(spamPool[i].ssid, final, len);
    spamPool[i].ssid_len = len;

    for (int j = 0; j < 6; j++) spamPool[i].bssid[j] = random(0, 256) & 0xFF;
    spamPool[i].bssid[0] = (spamPool[i].bssid[0] & 0xFE) | 0x02; // Local administered, unicast
  }
}

// Beacon Spammer RTOS Worker Task
void beaconSpamWorkerTask(void *pvParameters) {
  AttackCommand cmd;
  
  while (true) {
    // Check for incoming commands (Wait forever if idle, or don't block if active)
    if (xQueueReceive(beaconSpamQueue, &cmd, beaconSpamActive ? 0 : portMAX_DELAY) == pdTRUE) {
      if (cmd.action == 1) {
        beaconSpamActive = true;
        bsAttemptCount = 0;
        bsSuccessCount = 0;
        bsFailCount = 0;
        bsSequence = 0;
        bsCurrentChannel = 1;
        // Defensive: ensure promiscuous is still enabled on this core
        bool promiscState = false;
        esp_wifi_get_promiscuous(&promiscState);
        if (!promiscState) {
          esp_wifi_set_promiscuous(true);
          Serial.println("[BS-WORKER] Re-enabled promiscuous (was lost cross-core)");
        }
        Serial.println("[BS-WORKER] Beacon spam STARTED on Core 0");
        Serial.flush();
      } else if (cmd.action == 0) {
        beaconSpamActive = false;
        Serial.println("[BS-WORKER] Beacon spam STOPPED");
        Serial.flush();
      }
    }

    if (beaconSpamActive) {
      // 1. Channel Hopping (India: 1-13)
      bsCurrentChannel++;
      if (bsCurrentChannel > 13) bsCurrentChannel = 1;
      esp_wifi_set_channel(bsCurrentChannel, WIFI_SECOND_CHAN_NONE);

      // 2. Transmit Burst for current channel
      for (int i = 0; i < bsAPCount; i++) {
        // Yield check inside the loop to avoid stalling queue or core
        if (xQueueReceive(beaconSpamQueue, &cmd, 0) == pdTRUE) {
          if (cmd.action == 0) {
            beaconSpamActive = false;
            break; 
          }
        }

        // Build Frame dynamically based on pre-computed pool
        uint8_t ssidLen = spamPool[i].ssid_len;
        int frameSize = 38 + 2 + ssidLen + 3; // beacon_template size is 38
        uint8_t frame[128]; 
        
        memcpy(frame, beacon_template, 38);
        memcpy(&frame[10], spamPool[i].bssid, 6); // Source
        memcpy(&frame[16], spamPool[i].bssid, 6); // BSSID
        
        // Proper sequence number incrementing!
        frame[22] = (bsSequence & 0x0F) << 4;
        frame[23] = (bsSequence >> 4) & 0xFF;
        bsSequence++;
        
        int offset = 38;
        // SSID Element
        frame[offset++] = 0x00; // SSID Tag
        frame[offset++] = ssidLen;
        memcpy(&frame[offset], spamPool[i].ssid, ssidLen);
        offset += ssidLen;

        
        // DS Set Element (Channel)
        frame[offset++] = 0x03; // DS Parameter Set Tag
        frame[offset++] = 0x01; // Length
        frame[offset++] = bsCurrentChannel;

        // Transmit packet
        bsAttemptCount++;
        esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, frame, offset, false);
        if (result == ESP_OK) {
          bsSuccessCount++;
          bsSuccessCounterThisSecond++;
        } else {
          bsFailCount++;
        }
        // Log first TX result for diagnostics
        if (bsAttemptCount == 1) {
          Serial.printf("[TX-DIAG] First beacon spam TX result: 0x%x (%s)\n", result, esp_err_to_name(result));
          Serial.flush();
        }

        // Cooperative yield every 5 packets to breathe
        if (i % 5 == 0) {
          vTaskDelay(pdMS_TO_TICKS(5)); 
        }
      }

      // 3. Paced rest between channel hops (1000ms for stable RF behavior)
      vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
  }
}
