void handlewifimenu() {
  const char* menuItems[] = {"SCAN WIFI", "PACKET ANALYZER", "BEACON", "CAPTIVE PORTAL", "DEAUTH DETECTOR", "DEAUTHER", "BEACON SPAM"};
  const int menuLength = sizeof(menuItems) / sizeof(menuItems[0]);
  const int visibleItems = 3;

  static int selectedItem = 0;
  static int scrollOffset = 0;

  
  static unsigned long lastInputTime = 0;

  
  if (millis() - lastInputTime > 150) {
    


  if (digitalRead(BTN_UP) == LOW) {
    selectedItem--;
    if (selectedItem < 0) selectedItem = menuLength - 1;
    scrollOffset = constrain(selectedItem - visibleItems + 1, 0, menuLength - visibleItems);
    lastInputTime = millis(); 
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    selectedItem++;
    if (selectedItem >= menuLength) selectedItem = 0;
    scrollOffset = constrain(selectedItem - visibleItems + 1, 0, menuLength - visibleItems);
    lastInputTime = millis(); 
  }

  if (selectPressed()) {
    switch (selectedItem) {
      case 0:

     static const unsigned char image_file_search_bits[] U8X8_PROGMEM = {0x80,0x0f,0x40,0x10,0x20,0x20,0x10,0x40,0x10,0x40,0x10,0x50,0x10,0x50,0x10,0x48,0x20,0x26,0x50,0x10,0xa8,0x0f,0x14,0x00,0x0a,0x00,0x05,0x00,0x03,0x00,0x00,0x00};
static const unsigned char image_wifi_50_bits[] U8X8_PROGMEM = {0x80,0x0f,0x00,0x60,0x30,0x00,0x18,0xc0,0x00,0x84,0x0f,0x01,0x62,0x30,0x02,0x11,0x40,0x04,0x0a,0x87,0x02,0xc4,0x1f,0x01,0xe8,0xb8,0x00,0x70,0x77,0x00,0xa0,0x2f,0x00,0xc0,0x1d,0x00,0x80,0x0a,0x00,0x00,0x07,0x00,0x00,0x02,0x00,0x00,0x00,0x00};


    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    // file_search
    u8g2.drawXBMP(10, 21, 15, 16, image_file_search_bits);

    // wifi_50
    u8g2.drawXBMP(99, 20, 19, 16, image_wifi_50_bits);

    // Layer 3
    u8g2.setFont(u8g2_font_t0_13_tr);
    u8g2.drawStr(32, 15, "scanning");

    // Layer 4
    u8g2.drawStr(44, 32, "wifi ");

    // Layer 5
    u8g2.drawStr(33, 49, "networks");

    u8g2.sendBuffer();



       WiFi.mode(WIFI_STA);
       WiFi.disconnect();
       delay(100);
       wifi_networkCount = WiFi.scanNetworks();

      runLoop(scanningwifi);
      // Fix 3: Clean up WiFi scan results to prevent memory leak
      WiFi.scanDelete();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      delay(100);
       
        break;
      case 1:
       setupSnifferGraph(); 
       runLoop(updateSnifferGraph);
       // Fix 5: Cleanup promiscuous mode after sniffer exits
       esp_wifi_set_promiscuous(false);
       esp_wifi_set_promiscuous_rx_cb(NULL);
       WiFi.disconnect(true);
       WiFi.mode(WIFI_OFF);
       delay(100);
       break;
      case 2:
        beaconMode = true;
        {
          // Show scanning screen
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_t0_13_tr);
          u8g2.drawStr(32, 15, "scanning");
          u8g2.drawStr(44, 32, "wifi ");
          u8g2.drawStr(33, 49, "networks");
          u8g2.sendBuffer();
          WiFi.mode(WIFI_STA);
          WiFi.disconnect();
          delay(100);
          wifi_networkCount = WiFi.scanNetworks();
          runLoop(scanningwifi);
          
          WiFi.scanDelete();
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          delay(100);
        }
        beaconMode = false;
        break;
      case 3: 
      runLoop(loading);
       break;
      case 4:
      runLoop(loading);
        break;
      case 5:
        deauthMode = true;
        {
          // Show scanning screen
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_t0_13_tr);
          u8g2.drawStr(32, 15, "scanning");
          u8g2.drawStr(44, 32, "wifi ");
          u8g2.drawStr(33, 49, "networks");
          u8g2.sendBuffer();
          WiFi.mode(WIFI_STA);
          WiFi.disconnect();
          delay(100);
          wifi_networkCount = WiFi.scanNetworks();
          runLoop(scanningwifi);
          // Fix 3: Clean up after deauth scan too
          WiFi.scanDelete();
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          delay(100);
        }
        deauthMode = false;
        break;
      case 6: // BEACON SPAM
        runLoop(beaconSpamSetupUI);
        break;
    }
    lastInputTime = millis(); 
  }
  }
  // ===== عرض الشاشة =====
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tf); 

  for (int i = 0; i < visibleItems; i++) {
    int menuIndex = i + scrollOffset;
    if (menuIndex >= menuLength) break;

    int y = i * 20 + 16;

    if (menuIndex == selectedItem) {
      u8g2.drawRBox(4, y - 12, 120, 16, 4); 
      u8g2.setDrawColor(0); 
      u8g2.drawStr(10, y, menuItems[menuIndex]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(10, y, menuItems[menuIndex]);
    }
  }

  
  int barX = 124;
  int spacing = 64 / menuLength;

  for (int i = 0; i < menuLength; i++) {
    int dotY = i * spacing + spacing / 2;
    if (i == selectedItem) {
      u8g2.drawBox(barX, dotY - 3, 3, 6);
    } else {
      u8g2.drawPixel(barX + 1, dotY);
    }
  }

  u8g2.sendBuffer();
}

void beaconSpamRunningUI() {
  unsigned long now = millis();
  if (now - lastBsFpsUpdateTime >= 1000) {
    bsFps = bsSuccessCounterThisSecond;
    bsSuccessCounterThisSecond = 0;
    lastBsFpsUpdateTime = now;
  }
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawFrame(0, 0, 128, 14);
  u8g2.drawStr(4, 10, "BEACON SPAM ACTIVE");
  
  u8g2.setFont(u8g2_font_5x8_tr);
  char buf[32];
  snprintf(buf, sizeof(buf), "Mode: %d | APs: %d", bsMode, bsAPCount);
  u8g2.drawStr(4, 25, buf);
  
  snprintf(buf, sizeof(buf), "Chan: %d | Seq: %d", bsCurrentChannel, bsSequence);
  u8g2.drawStr(4, 35, buf);
  
  // Animation
  static int animState = 0;
  static unsigned long lastAnimTime = 0;
  if (now - lastAnimTime > 250) {
    animState = (animState + 1) % 4;
    lastAnimTime = now;
  }
  char animBuf[16] = "SPAMMING";
  for (int i = 0; i < animState; i++) animBuf[8 + i] = '.';
  animBuf[8 + animState] = '\0';
  u8g2.drawStr(4, 45, animBuf);
  
  snprintf(buf, sizeof(buf), "Tx:%lu Ok:%lu %lu/s", bsAttemptCount, bsSuccessCount, bsFps);
  u8g2.drawStr(4, 55, buf);
  
  u8g2.sendBuffer();
}

void beaconSpamSetupUI() {
  static const char* bsModes[] = {"Common Names", "Random Garbage", "Rick Roll", "Troll Names"};
  static int sel = 0;
  
  if (digitalRead(BTN_UP) == LOW) {
    sel--; if(sel < 0) sel = 3;
    delay(150); // debounce
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    sel++; if(sel > 3) sel = 0;
    delay(150); // debounce
  }
  
  if (selectPressed()) {
    bsMode = sel;
    // Set to STA mode + Promiscuous to allow beacon transmission
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_STA);
    delay(100);
    esp_wifi_set_promiscuous(true);
    
    // Generate pool
    generateSpamAPs(bsMode, bsAPCount);
    
    // Send Start command
    AttackCommand startCmd; startCmd.action = 1;
    xQueueSend(beaconSpamQueue, &startCmd, portMAX_DELAY);
    
    // Wait for button release
    while(digitalRead(BTN_SELECT) == LOW) delay(10);
    
    // Launch running UI (blocks until BACK is pressed)
    runLoop(beaconSpamRunningUI);
    
    // Once returned, send Stop command
    AttackCommand stopCmd; stopCmd.action = 0;
    xQueueSend(beaconSpamQueue, &stopCmd, portMAX_DELAY);
    
    // Wait for BACK button release
    while(digitalRead(BTN_BACK) == LOW) delay(10);
    
    // Restore WiFi state
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_STA);
    delay(150);
  }
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(0, 10, "Select Beacon Mode:");
  for (int i=0; i<4; i++) {
    if (i == sel) {
      u8g2.drawBox(0, 15 + (i*10), 128, 10);
      u8g2.setDrawColor(0);
      u8g2.drawStr(5, 23 + (i*10), bsModes[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(5, 23 + (i*10), bsModes[i]);
    }
  }
  u8g2.sendBuffer();
}
