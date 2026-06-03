
   





void scanningwifi() {
  if (wifi_networkCount == -1) {
    // --- Async Scanning Animation & Completion Polling ---
    u8g2.clearBuffer();
    
    // Draw scanning text
    u8g2.setFont(u8g2_font_t0_13_tr);
    u8g2.drawStr(32, 15, "scanning");
    u8g2.drawStr(44, 32, "wifi ");
    u8g2.drawStr(33, 49, "networks");
    
    // Draw an animated loading bar at the bottom
    int progressWidth = (millis() / 8) % 120;
    u8g2.drawFrame(4, 56, 120, 6);
    u8g2.drawBox(6, 58, progressWidth, 2);
    
    u8g2.sendBuffer();
    
    // Poll the async scan state
    int16_t result = WiFi.scanComplete();
    if (result >= 0) {
      wifi_networkCount = result;
      wifi_selectedIndex = 0;
      wifi_showInfo = false;
      Serial.printf("WiFi Async scan complete. Found %d networks.\n", result);
    } else if (result == WIFI_SCAN_FAILED) {
      wifi_networkCount = -2; // Scan failed flag
      Serial.println("WiFi Async scan failed!");
    }
    return;
  }
  
  if (wifi_networkCount == -2) {
    // Scan failed display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_13_tr);
    u8g2.drawStr(25, 25, "Scan Failed!");
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(15, 45, "[Press BACK to return]");
    u8g2.sendBuffer();
    return;
  }
  
  if (wifi_networkCount == 0) {
    // No networks found display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_13_tr);
    u8g2.drawStr(12, 25, "No Networks Found!");
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(15, 45, "[Press BACK to return]");
    u8g2.sendBuffer();
    return;
  }

  // --- Normal Scan Results Navigation & Rendering ---
  // Constrain selected index just to be absolutely safe
  if (wifi_selectedIndex < 0) wifi_selectedIndex = 0;
  if (wifi_selectedIndex >= wifi_networkCount) wifi_selectedIndex = wifi_networkCount - 1;

  if (digitalRead(BTN_DOWN) == LOW) {
    wifi_selectedIndex++;
    if (wifi_selectedIndex >= wifi_networkCount) wifi_selectedIndex = 0;
    delay(50);
  }

  if (digitalRead(BTN_UP) == LOW) {
    wifi_selectedIndex--;
    if (wifi_selectedIndex < 0) wifi_selectedIndex = wifi_networkCount - 1;
    delay(50);
  }

  if (digitalRead(BTN_SELECT) == LOW) {
    wifi_showInfo = true;
    delay(50);
  }

  if (digitalRead(BTN_BACK) == LOW) {
    wifi_showInfo = false;
    delay(50);
  }

  u8g2.clearBuffer();

  if (!wifi_showInfo) {
    setColor(0, 0, 0);
    // ----- Main WiFi List -----
    u8g2.setFont(u8g2_font_6x13B_tr);  // Slightly smaller but bold font

    int start = max(0, wifi_selectedIndex - 2);
    int end = min(start + 4, wifi_networkCount);

    for (int i = start; i < end; i++) {
      int y = (i - start) * 16 + 2;
      
      // Safety bounds check before calling SSID(i)
      String ssidStr = (i >= 0 && i < wifi_networkCount) ? WiFi.SSID(i) : "";
      
      if (i == wifi_selectedIndex) {
        u8g2.setDrawColor(1);
        u8g2.drawRBox(0, y, 128, 16, 4);  // Highlight with rounded rectangle
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, y + 12, ssidStr.c_str());
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, y + 12, ssidStr.c_str());
      }
    }

  } else {
    // ----- Detailed WiFi Info -----
    u8g2.setFont(u8g2_font_5x8_tr);  // Smaller font to show full MAC address
    u8g2.setDrawColor(1);

    // Double-check index safety before accessing properties
    if (wifi_selectedIndex >= 0 && wifi_selectedIndex < wifi_networkCount) {
      // Box 1: SSID
      u8g2.drawFrame(0, 0, 128, 13);
      u8g2.drawStr(4, 9, ("SSID: " + WiFi.SSID(wifi_selectedIndex)).c_str());

      // Box 2: RSSI
      u8g2.drawFrame(0, 14, 128, 13);
      u8g2.drawStr(4, 23, ("RSSI: " + String(WiFi.RSSI(wifi_selectedIndex)) + " dBm").c_str());

      // Box 3: MAC
      u8g2.drawFrame(0, 28, 128, 13);
      u8g2.drawStr(4, 37, ("MAC: " + WiFi.BSSIDstr(wifi_selectedIndex)).c_str());

      // Box 4: Encryption
      u8g2.drawFrame(0, 42, 128, 13);
      u8g2.drawStr(4, 51, ("Enc: " + wifi_encryptionType(WiFi.encryptionType(wifi_selectedIndex))).c_str());
    }

    // Box 5: Back hint
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(38, 62, "[اضغط BACK للرجوع]");
  }

  u8g2.sendBuffer();
}

String wifi_encryptionType(wifi_auth_mode_t encryption) {
  switch (encryption) {
    case WIFI_AUTH_OPEN: return "Open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    default: return "Unknown";
  }
}

