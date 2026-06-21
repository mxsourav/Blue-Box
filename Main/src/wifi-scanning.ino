
   





void scanningwifi() {
  if (wifi_networkCount <= 0) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(10, 25, "No networks found");
    u8g2.drawStr(10, 40, "or scan failed.");
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 55, "[BACK] to return");
    u8g2.sendBuffer();
    wifi_selectedIndex = 0;
    return;
  }


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
    // If in deauth mode and showing info, set target and launch deauther
    if (deauthMode && wifi_showInfo) {
      attackTargetSSID = WiFi.SSID(wifi_selectedIndex);
      attackTargetMACStr = WiFi.BSSIDstr(wifi_selectedIndex);
      attackTargetChannel = WiFi.channel(wifi_selectedIndex);
      deauthActive = false;
      runLoop(deautherAttackLoop);
      // Cleanup: turn off promiscuous, WiFi, and LED after attack exits
      deauthActive = false;
      setColor(0, 0, 0);
      esp_wifi_set_promiscuous(false);
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      delay(100);
      Serial.println("[TX-EXIT] Deauther stopped, promiscuous OFF, WiFi OFF");
      wifi_showInfo = false;
    }
    // If in beacon mode and showing info, set target and launch beacon cloning
    else if (beaconMode && wifi_showInfo) {
      attackTargetSSID = WiFi.SSID(wifi_selectedIndex);
      attackTargetMACStr = WiFi.BSSIDstr(wifi_selectedIndex);
      attackTargetChannel = WiFi.channel(wifi_selectedIndex);
      beaconActive = false;
      runLoop(beaconAttackLoop);
      
      beaconActive = false;
      setColor(0, 0, 0);
      esp_wifi_set_promiscuous(false);
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      delay(100);
      Serial.println("[TX-EXIT] Beacon clone stopped, promiscuous OFF, WiFi OFF");
      wifi_showInfo = false;
    }
    delay(50);
  }

  if (digitalRead(BTN_BACK) == LOW) {
    wifi_showInfo = false;
    delay(50);
  }

  u8g2.clearBuffer();

  if (!wifi_showInfo) {

    setColor(0,0,0);
    // ----- Main WiFi List -----
    u8g2.setFont(u8g2_font_6x13B_tr);  // Slightly smaller but bold font

    int start = max(0, wifi_selectedIndex - 2);
    int end = min(start + 4, wifi_networkCount);

    for (int i = start; i < end; i++) {
      int y = (i - start) * 16 + 2;
      // Fix 7: Cache SSID to avoid creating String temporaries every 30ms
      String ssidStr = WiFi.SSID(i);
      const char* ssid = ssidStr.c_str();
      if (i == wifi_selectedIndex) {
        u8g2.setDrawColor(1);
        u8g2.drawRBox(0, y, 128, 16, 4);  // Highlight with rounded rectangle
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, y + 12, ssid);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, y + 12, ssid);
      }
    }

  } else {
    // ----- Detailed WiFi Info -----
    u8g2.setFont(u8g2_font_5x8_tr);  // Smaller font to show full MAC address
    u8g2.setDrawColor(1);

    // Fix 7: Use char buffers instead of String concatenation in hot loop
    char infoBuf[32];
    
    // Box 1: SSID
    u8g2.drawFrame(0, 0, 128, 13);
    snprintf(infoBuf, sizeof(infoBuf), "SSID: %s", WiFi.SSID(wifi_selectedIndex).c_str());
    u8g2.drawStr(4, 9, infoBuf);

    // Box 2: RSSI
    u8g2.drawFrame(0, 14, 128, 13);
    snprintf(infoBuf, sizeof(infoBuf), "RSSI: %d dBm", WiFi.RSSI(wifi_selectedIndex));
    u8g2.drawStr(4, 23, infoBuf);

    // Box 3: MAC
    u8g2.drawFrame(0, 28, 128, 13);
    snprintf(infoBuf, sizeof(infoBuf), "MAC: %s", WiFi.BSSIDstr(wifi_selectedIndex).c_str());
    u8g2.drawStr(4, 37, infoBuf);

    // Box 4: Encryption
    u8g2.drawFrame(0, 42, 128, 13);
    snprintf(infoBuf, sizeof(infoBuf), "Enc: %s", wifi_encryptionType(WiFi.encryptionType(wifi_selectedIndex)).c_str());
    u8g2.drawStr(4, 51, infoBuf);

    // Box 5: Back hint / Deauth option
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.setFont(u8g2_font_4x6_tr);
    if (deauthMode) {
      u8g2.drawStr(10, 62, "[SELECT: DEAUTH  BACK: EXIT]");
    } else if (beaconMode) {
      u8g2.drawStr(10, 62, "[SELECT: BEACON  BACK: EXIT]");
    } else {
      u8g2.drawStr(38, 62, "[BACK to return]");
    }
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

