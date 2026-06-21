// ============================================================
// wifi_scanner.ino — WiFi Network Scanner for Blue-Box
// Ported from Bruce firmware's wifi_atks.cpp scan logic
// Display rewritten for U8G2 128x64 monochrome OLED
// ============================================================
// NOTE: WiFiNetwork struct, scannedNets[], and wifi_netCount
// are defined in wifi_deauth.ino (alphabetically first)

// ===== Perform WiFi Scan =====
void wifiDoScan() {
  Serial.println("[WIFI] Starting scan...");
  
  // Show scanning screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.drawStr(20, 25, "Scanning");
  u8g2.drawStr(20, 42, "WiFi Networks...");
  u8g2.sendBuffer();
  
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(8); // Limit TX power to 2dBm to prevent brownouts
  WiFi.disconnect();
  delay(100);
  
  int found = WiFi.scanNetworks(false, true); // include hidden
  wifi_netCount = min(found, WIFI_MAX_NETWORKS);
  
  // Copy results to our fixed-size array
  for (int i = 0; i < wifi_netCount; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      snprintf(scannedNets[i].ssid, 33, "<Hidden %s>", WiFi.BSSIDstr(i).substring(9).c_str());
    } else {
      strncpy(scannedNets[i].ssid, ssid.c_str(), 32);
      scannedNets[i].ssid[32] = '\0';
    }
    memcpy(scannedNets[i].bssid, WiFi.BSSID(i), 6);
    scannedNets[i].rssi = WiFi.RSSI(i);
    scannedNets[i].channel = (uint8_t)WiFi.channel(i);
    scannedNets[i].encryption = WiFi.encryptionType(i);
  }
  
  // Free scan results from ESP-IDF memory
  WiFi.scanDelete();
  
  wifi_selIndex = 0;
  wifi_scrollOffset = 0;
  wifi_inDetail = false;
  wifi_inAttackMenu = false;
  
  Serial.printf("[WIFI] Found %d networks\n", wifi_netCount);
}

// ===== Draw RSSI Bar (visual signal strength) =====
void drawRssiBar(int x, int y, int32_t rssi) {
  // Map RSSI to 0-4 bars: -90 or worse = 0, -50 or better = 4
  int bars = 0;
  if (rssi > -55) bars = 4;
  else if (rssi > -65) bars = 3;
  else if (rssi > -75) bars = 2;
  else if (rssi > -85) bars = 1;
  
  for (int i = 0; i < 4; i++) {
    int bh = (i + 1) * 2; // bar height: 2,4,6,8
    int by = y + 8 - bh;
    if (i < bars) {
      u8g2.drawBox(x + i * 3, by, 2, bh);
    } else {
      u8g2.drawPixel(x + i * 3, y + 7);
    }
  }
}

// ===== Draw Network List (main scan view) =====
void wifiDrawNetworkList() {
  u8g2.clearBuffer();
  
  if (wifi_netCount <= 0) {
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(8, 25, "No networks");
    u8g2.drawStr(8, 42, "found.");
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(8, 58, "[BACK] return [SEL] rescan");
    u8g2.sendBuffer();
    return;
  }
  
  // Title bar
  u8g2.setFont(u8g2_font_5x7_tf);
  char titleBuf[32];
  snprintf(titleBuf, 32, "WiFi Scan: %d/%d", wifi_selIndex + 1, wifi_netCount);
  u8g2.drawStr(0, 7, titleBuf);
  u8g2.drawHLine(0, 9, 128);
  
  // List items (4 visible, 13px each)
  const int listTop = 12;
  const int itemH = 13;
  const int maxVisible = 4;
  
  // Adjust scroll
  if (wifi_selIndex < wifi_scrollOffset) wifi_scrollOffset = wifi_selIndex;
  if (wifi_selIndex >= wifi_scrollOffset + maxVisible) wifi_scrollOffset = wifi_selIndex - maxVisible + 1;
  
  u8g2.setFont(u8g2_font_6x10_tf);
  for (int i = 0; i < maxVisible; i++) {
    int idx = wifi_scrollOffset + i;
    if (idx >= wifi_netCount) break;
    
    int y = listTop + i * itemH;
    
    // Highlight selected
    if (idx == wifi_selIndex) {
      u8g2.drawRBox(0, y, 116, itemH, 2);
      u8g2.setDrawColor(0);
    }
    
    // Lock icon for encrypted
    if (scannedNets[idx].encryption != WIFI_AUTH_OPEN) {
      u8g2.drawStr(2, y + 10, "#");
    }
    
    // SSID (truncated to fit)
    char dispSSID[18];
    strncpy(dispSSID, scannedNets[idx].ssid, 17);
    dispSSID[17] = '\0';
    u8g2.drawStr(10, y + 10, dispSSID);
    
    // RSSI bar
    if (idx == wifi_selIndex) u8g2.setDrawColor(0);
    drawRssiBar(104, y + 1, scannedNets[idx].rssi);
    
    u8g2.setDrawColor(1);
  }
  
  // Scrollbar
  if (wifi_netCount > maxVisible) {
    int barH = max(4, (maxVisible * (maxVisible * itemH)) / wifi_netCount);
    int barY = listTop + (wifi_scrollOffset * (maxVisible * itemH - barH)) / max(1, wifi_netCount - maxVisible);
    u8g2.drawBox(126, barY, 2, barH);
  }
  
  // Bottom hint
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 63, "[SEL]info [BACK]exit");
  
  u8g2.sendBuffer();
}

// ===== Draw Network Detail Screen =====
void wifiDrawNetworkDetail(int idx) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  
  WiFiNetwork &net = scannedNets[idx];
  
  // SSID
  char line[32];
  snprintf(line, 32, "%.20s", net.ssid);
  u8g2.drawStr(0, 8, line);
  u8g2.drawHLine(0, 10, 128);
  
  // BSSID
  u8g2.drawStr(0, 19, wifiMacToString(net.bssid).c_str());
  
  // Channel + Encryption
  snprintf(line, 32, "CH:%d  %s", net.channel, wifiEncryptionString(net.encryption).c_str());
  u8g2.drawStr(0, 28, line);
  
  // RSSI
  snprintf(line, 32, "RSSI: %d dBm", net.rssi);
  u8g2.drawStr(0, 37, line);
  
  // Signal quality bar
  int quality = min(100, max(0, (int)(2 * (net.rssi + 100))));
  int barW = (quality * 100) / 100;
  u8g2.drawFrame(0, 40, 102, 8);
  u8g2.drawBox(1, 41, barW, 6);
  snprintf(line, 32, "%d%%", quality);
  u8g2.drawStr(106, 47, line);
  
  // Action hints
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 56, "[SEL] Attack Menu");
  u8g2.drawStr(0, 63, "[BACK] Return to list");
  
  u8g2.sendBuffer();
}

// ===== Draw Attack Sub-menu for selected network =====
void wifiDrawAttackMenu(int idx) {
  static int atkSel = 0;
  const char* atkItems[] = {"DEAUTH TARGET", "DEAUTH FLOOD", "BACK"};
  const int atkCount = 3;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Title
  char title[24];
  snprintf(title, 24, "Attack: %.12s", scannedNets[idx].ssid);
  u8g2.drawStr(0, 10, title);
  u8g2.drawHLine(0, 12, 128);
  
  // Menu items
  for (int i = 0; i < atkCount; i++) {
    int y = 16 + i * 16;
    if (i == atkSel) {
      u8g2.drawRBox(4, y, 120, 14, 3);
      u8g2.setDrawColor(0);
      u8g2.drawStr(10, y + 11, atkItems[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(10, y + 11, atkItems[i]);
    }
  }
  u8g2.sendBuffer();
  
  // Handle input
  static unsigned long lastInput = 0;
  if (millis() - lastInput > 150) {
    if (digitalRead(BTN_UP) == LOW) {
      atkSel = (atkSel - 1 + atkCount) % atkCount;
      lastInput = millis();
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      atkSel = (atkSel + 1) % atkCount;
      lastInput = millis();
    }
    if (digitalRead(BTN_SELECT) == LOW) {
      lastInput = millis();
      switch (atkSel) {
        case 0: // Deauth Target
          deauthTarget(idx);
          wifi_inAttackMenu = false;
          wifi_inDetail = false;
          break;
        case 1: // Deauth Flood
          deauthFlood();
          wifi_inAttackMenu = false;
          wifi_inDetail = false;
          break;
        case 2: // Back
          wifi_inAttackMenu = false;
          break;
      }
      atkSel = 0;
    }
  }
}

// ===== Main Scanner Loop Function (called by runLoop) =====
void wifiScannerLoop() {
  static unsigned long lastInput = 0;
  
  if (wifi_inAttackMenu) {
    wifiDrawAttackMenu(wifi_selIndex);
    return;
  }
  
  if (wifi_inDetail) {
    wifiDrawNetworkDetail(wifi_selIndex);
    
    if (millis() - lastInput > 150) {
      if (digitalRead(BTN_SELECT) == LOW) {
        wifi_inAttackMenu = true;
        lastInput = millis();
      }
      if (digitalRead(BTN_BACK) == LOW) {
        wifi_inDetail = false;
        lastInput = millis();
      }
    }
    return;
  }
  
  // Network list view
  wifiDrawNetworkList();
  
  if (millis() - lastInput > 150) {
    if (digitalRead(BTN_DOWN) == LOW && wifi_netCount > 0) {
      wifi_selIndex = (wifi_selIndex + 1) % wifi_netCount;
      lastInput = millis();
    }
    if (digitalRead(BTN_UP) == LOW && wifi_netCount > 0) {
      wifi_selIndex = (wifi_selIndex - 1 + wifi_netCount) % wifi_netCount;
      lastInput = millis();
    }
    if (digitalRead(BTN_SELECT) == LOW) {
      if (wifi_netCount <= 0) {
        // Rescan
        wifiDoScan();
      } else {
        wifi_inDetail = true;
      }
      lastInput = millis();
    }
  }
}
