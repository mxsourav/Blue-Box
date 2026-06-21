// ============================================================
// wifi_utils.ino — WiFi utility functions for Blue-Box
// Ported from Bruce firmware's net_utils.cpp & wifi_common.cpp
// ============================================================
// NOTE: WiFi.h and esp_wifi.h are included in main .ino

// ===== MAC / IP string conversions =====

String wifiMacToString(const uint8_t *mac) {
  char buf[18];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void wifiStringToMAC(const String &macStr, uint8_t mac[6]) {
  int values[6];
  if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
             &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5]) == 6) {
    for (int i = 0; i < 6; i++) mac[i] = (uint8_t)values[i];
  } else {
    memset(mac, 0, 6);
  }
}

String wifiIpToString(const uint8_t *ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

String wifiEncryptionString(wifi_auth_mode_t enc) {
  switch (enc) {
    case WIFI_AUTH_OPEN:            return "Open";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "Enterprise";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3";
    default:                        return "?";
  }
}

// ===== WiFi Setup / Cleanup =====

// Override the raw frame sanity check to allow deauth TX
// (This is also in deauth_override.c but we keep it here as extern "C" backup)
// extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
//     return 1;
// }

bool wifiSetupAttackMode() {
  Serial.println("[WIFI] Setting up attack mode (APSTA)...");
  
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  // Start a hidden soft AP for raw frame TX
  if (!WiFi.softAP("BlueBoxAtk", "", 1, 1, 4, false)) {
    Serial.println("[WIFI] Failed to start AP mode");
    return false;
  }
  delay(100);
  
  // Limit TX power to prevent brownouts
  esp_wifi_set_max_tx_power(8);
  
  Serial.println("[WIFI] Attack mode ready");
  return true;
}

void wifiCleanup() {
  Serial.println("[WIFI] Cleanup...");
  esp_wifi_set_promiscuous(false);
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  delay(50);
  Serial.println("[WIFI] Cleanup done");
}

// ===== Raw frame transmission (from Bruce's send_raw_frame) =====

// Send a raw 802.11 frame, returns true if at least one TX succeeded
// Sends 3 copies with small delays (Bruce's proven pattern)
bool wifiSendRawFrame(const uint8_t *frame, int size) {
  esp_err_t e1 = esp_wifi_80211_tx(WIFI_IF_AP, frame, size, false);
  delay(1);
  esp_err_t e2 = esp_wifi_80211_tx(WIFI_IF_AP, frame, size, false);
  delay(1);
  esp_err_t e3 = esp_wifi_80211_tx(WIFI_IF_AP, frame, size, false);
  delay(1);
  return (e1 == ESP_OK || e2 == ESP_OK || e3 == ESP_OK);
}

// Single raw frame TX with result tracking
bool wifiSendSingleFrame(const uint8_t *frame, int size, wifi_interface_t iface) {
  esp_err_t err = esp_wifi_80211_tx(iface, frame, size, false);
  return (err == ESP_OK);
}

// ===== Monitor / Promiscuous mode helpers =====

bool wifiEnterMonitorMode(uint8_t channel) {
  Serial.printf("[WIFI] Entering monitor mode on CH%d\n", channel);
  
  // Stop WiFi briefly
  esp_wifi_stop();
  delay(10);
  
  // Reinitialize
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  
  // Set STA mode (required for promiscuous)
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
  delay(10);
  
  // Enable promiscuous mode with all packet types
  wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
  };
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous(true);
  
  // Set channel
  esp_err_t err = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  if (err != ESP_OK) {
    Serial.printf("[WIFI] Failed to set channel %d: %d\n", channel, err);
    esp_wifi_set_promiscuous(false);
    return false;
  }
  
  // Limit TX power to prevent brownouts
  esp_wifi_set_max_tx_power(8);
  
  Serial.printf("[WIFI] Monitor mode active on CH%d\n", channel);
  return true;
}

void wifiExitMonitorMode() {
  esp_wifi_set_promiscuous(false);
  delay(10);
  WiFi.mode(WIFI_STA);
  delay(50);
}
