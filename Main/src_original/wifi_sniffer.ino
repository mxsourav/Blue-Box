// ============================================================
// wifi_sniffer.ino — Raw 802.11 Packet Sniffer for Blue-Box
// Inspired by Bruce firmware's sniffer.cpp
// Display adapted for U8G2 128x64 monochrome OLED
// Real-time packet monitoring with channel hopping
// ============================================================
// NOTE: WiFi.h and esp_wifi.h are included in main .ino

// ===== Sniffer State =====
volatile uint32_t sniff_pktTotal = 0;
volatile uint32_t sniff_pktMgmt = 0;
volatile uint32_t sniff_pktData = 0;
volatile uint32_t sniff_pktCtrl = 0;
volatile uint32_t sniff_pktMisc = 0;
volatile uint32_t sniff_pktDeauth = 0;
volatile uint32_t sniff_pktBeacon = 0;
volatile uint32_t sniff_pktProbe = 0;

uint32_t sniff_ppsHistory[128]; // Packets per second rolling graph (128 columns)
int sniff_histIdx = 0;
uint32_t sniff_ppsCounter = 0;
unsigned long sniff_lastPpsTime = 0;
uint32_t sniff_currentPps = 0;
uint8_t sniff_channel = 1;
bool sniff_channelHop = true;
unsigned long sniff_lastHop = 0;
bool sniff_running = false;
volatile uint32_t sniff_pktDropped = 0; // ISR queue-full drops

// ===== Sniffer Asynchronous Queue & Task =====
typedef struct {
  uint8_t payload[64];
  uint16_t length;
  int8_t rssi;
  uint8_t channel;
  uint8_t type;
} SnifferQueueItem;

QueueHandle_t snifferQueue = NULL;
TaskHandle_t snifferTaskHandle = NULL;
void snifferWorkerTask(void* pvParameters);

const uint8_t sniff_channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const int sniff_numChannels = 11;
int sniff_chanIdx = 0;

// ===== Promiscuous callback (ISR context — keep it FAST) =====
void IRAM_ATTR snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!sniff_running || snifferQueue == NULL) return;
  
  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  SnifferQueueItem item;
  
  item.length = (pkt->rx_ctrl.sig_len < 64) ? pkt->rx_ctrl.sig_len : 64;
  item.rssi = pkt->rx_ctrl.rssi;
  item.channel = sniff_channel;
  item.type = type;
  memcpy(item.payload, pkt->payload, item.length);
  
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xQueueSendFromISR(snifferQueue, &item, &xHigherPriorityTaskWoken) != pdTRUE) {
    sniff_pktDropped++; // Queue full — drop silently, no prints in ISR
  }
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

// ===== Sniffer Worker Task (Processes packets asynchronously on Core 0) =====
void snifferWorkerTask(void* pvParameters) {
  SnifferQueueItem item;
  while (true) {
    if (xQueueReceive(snifferQueue, &item, portMAX_DELAY) == pdTRUE) {
      sniff_pktTotal++;
      sniff_ppsCounter++;
      
      switch (item.type) {
        case WIFI_PKT_MGMT: {
          sniff_pktMgmt++;
          uint8_t subtype = (item.payload[0] >> 4) & 0x0F;
          if (subtype == 0x0C || subtype == 0x0A) sniff_pktDeauth++; // Deauth / Disassoc
          else if (subtype == 0x08) sniff_pktBeacon++;                 // Beacon
          else if (subtype == 0x04) sniff_pktProbe++;                  // Probe Request
          break;
        }
        case WIFI_PKT_DATA:
          sniff_pktData++;
          break;
        case WIFI_PKT_CTRL:
          sniff_pktCtrl++;
          break;
        default:
          sniff_pktMisc++;
          break;
      }
    }
  }
}

// ===== Start Sniffer =====
void snifferStart() {
  Serial.println("[SNIFFER] Starting...");
  
  // Reset counters
  sniff_pktTotal = 0;
  sniff_pktMgmt = 0;
  sniff_pktData = 0;
  sniff_pktCtrl = 0;
  sniff_pktMisc = 0;
  sniff_pktDeauth = 0;
  sniff_pktBeacon = 0;
  sniff_pktProbe = 0;
  sniff_ppsCounter = 0;
  sniff_currentPps = 0;
  sniff_lastPpsTime = millis();
  sniff_lastHop = millis();
  sniff_chanIdx = 0;
  sniff_channel = 1;
  sniff_histIdx = 0;
  memset(sniff_ppsHistory, 0, sizeof(sniff_ppsHistory));
  
  // 1. Initialize WiFi mode STA first (powers on RF stack)
  Serial.println("[SNIFFER] Calling WiFi.mode(WIFI_STA)...");
  Serial.flush();
  WiFi.mode(WIFI_STA);
  Serial.println("[SNIFFER] WiFi.mode(WIFI_STA) Done");
  Serial.flush();
  delay(100); // Allow RF power transient to settle
  
  // 2. Limit TX power immediately to prevent brownouts/resets
  Serial.println("[SNIFFER] Setting TX power...");
  Serial.flush();
  esp_wifi_set_max_tx_power(8);
  Serial.println("[SNIFFER] TX power set to 8");
  Serial.flush();
  delay(50);
  
  // 3. Create Queue if not exists
  Serial.println("[SNIFFER] Creating Queue...");
  Serial.flush();
  if (snifferQueue == NULL) {
    snifferQueue = xQueueCreate(64, sizeof(SnifferQueueItem));
  }
  Serial.println("[SNIFFER] Queue Ready");
  Serial.flush();
  
  // 4. Spawn Worker Task on Core 0
  Serial.println("[SNIFFER] Spawning worker task...");
  Serial.flush();
  if (snifferTaskHandle == NULL) {
    xTaskCreatePinnedToCore(
      snifferWorkerTask,
      "snifferWorkerTask",
      4096,
      NULL,
      2,
      &snifferTaskHandle,
      0 // Core 0
    );
  }
  Serial.println("[SNIFFER] Worker task running");
  Serial.flush();
  
  // 5. Enable promiscuous mode
  Serial.println("[SNIFFER] Setting promiscuous false...");
  Serial.flush();
  esp_wifi_set_promiscuous(false);
  Serial.println("[SNIFFER] Setting filter...");
  Serial.flush();
  
  wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
  };
  esp_wifi_set_promiscuous_filter(&filter);
  Serial.println("[SNIFFER] Setting callback...");
  Serial.flush();
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);
  Serial.println("[SNIFFER] Setting promiscuous true...");
  Serial.flush();
  esp_wifi_set_promiscuous(true);
  Serial.println("[SNIFFER] Setting channel...");
  Serial.flush();
  
  esp_wifi_set_channel(sniff_channel, WIFI_SECOND_CHAN_NONE);
  
  sniff_running = true;
  Serial.println("[SNIFFER] Active on CH1");
  Serial.flush();
}

// ===== Stop Sniffer =====
void snifferStop() {
  sniff_running = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(NULL);
  
  // Terminate background worker task
  if (snifferTaskHandle != NULL) {
    vTaskDelete(snifferTaskHandle);
    snifferTaskHandle = NULL;
  }
  
  // Drain and delete queue
  if (snifferQueue != NULL) {
    vQueueDelete(snifferQueue);
    snifferQueue = NULL;
  }
  
  delay(10);
  WiFi.mode(WIFI_STA);
  Serial.printf("[SNIFFER] Stopped. Total: %lu packets\n", sniff_pktTotal);
}

// ===== Sniffer Display Loop (called by handlewifimenu) =====
void snifferLoop() {
  static unsigned long lastInput = 0;
  
  // Channel hopping
  if (sniff_channelHop && millis() - sniff_lastHop > 500) {
    sniff_chanIdx = (sniff_chanIdx + 1) % sniff_numChannels;
    sniff_channel = sniff_channels[sniff_chanIdx];
    esp_wifi_set_channel(sniff_channel, WIFI_SECOND_CHAN_NONE);
    sniff_lastHop = millis();
  }
  
  // Calculate PPS every second
  if (millis() - sniff_lastPpsTime >= 1000) {
    sniff_currentPps = sniff_ppsCounter;
    sniff_ppsCounter = 0;
    sniff_lastPpsTime = millis();
    
    // Add to history graph
    sniff_ppsHistory[sniff_histIdx] = sniff_currentPps;
    sniff_histIdx = (sniff_histIdx + 1) % 128;
    
    // === RUNTIME DIAGNOSTICS (every 1s) ===
    UBaseType_t qFill = (snifferQueue != NULL) ? uxQueueMessagesWaiting(snifferQueue) : 0;
    UBaseType_t stackHWM = (snifferTaskHandle != NULL) ? uxTaskGetStackHighWaterMark(snifferTaskHandle) : 0;
    Serial.printf("[DIAG] Heap:%lu MaxBlk:%lu QFill:%u/%d StkHWM:%u Drops:%lu PPS:%lu Tot:%lu\n",
      (unsigned long)esp_get_free_heap_size(),
      (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
      (unsigned int)qFill, 64,
      (unsigned int)stackHWM,
      (unsigned long)sniff_pktDropped,
      (unsigned long)sniff_currentPps,
      (unsigned long)sniff_pktTotal);
  }
  static unsigned long lastDraw = 0;
  static uint32_t lastPktCount = 0xFFFFFFFF;
  static bool lastHopState = false;
  static uint8_t lastChan = 0;
  
  bool stateChanged = (sniff_pktTotal != lastPktCount) || 
                      (sniff_channelHop != lastHopState) || 
                      (sniff_channel != lastChan);
                      
  if (stateChanged && (millis() - lastDraw >= 333)) {
    lastDraw = millis();
    lastPktCount = sniff_pktTotal;
    lastHopState = sniff_channelHop;
    lastChan = sniff_channel;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);
    
    // Header: Channel + PPS
    char line[32];
    snprintf(line, 32, "CH:%d  %lu pkt/s", sniff_channel, sniff_currentPps);
    u8g2.drawStr(0, 7, line);
    
    // Hop indicator
    u8g2.drawStr(108, 7, sniff_channelHop ? "HOP" : "FIX");
    u8g2.drawHLine(0, 9, 128);
    
    // Scrolling packet graph (128 columns, height 24px, y range 10-33)
    const int graphTop = 10;
    const int graphH = 24;
    
    // Find max for scaling
    uint32_t maxPps = 1;
    for (int i = 0; i < 128; i++) {
      if (sniff_ppsHistory[i] > maxPps) maxPps = sniff_ppsHistory[i];
    }
    
    // Draw graph bars
    for (int i = 0; i < 128; i++) {
      int idx = (sniff_histIdx + i) % 128;
      if (sniff_ppsHistory[idx] > 0) {
        int barH = max(1, (int)((float)sniff_ppsHistory[idx] / maxPps * graphH));
        int barY = graphTop + graphH - barH;
        u8g2.drawVLine(i, barY, barH);
      }
    }
    
    // Bottom separator
    u8g2.drawHLine(0, graphTop + graphH + 1, 128);
    
    // Packet type breakdown
    int infoY = graphTop + graphH + 9;
    snprintf(line, 32, "M:%lu D:%lu C:%lu", sniff_pktMgmt, sniff_pktData, sniff_pktCtrl);
    u8g2.drawStr(0, infoY, line);
    
    // Deauth/Beacon counters
    infoY += 8;
    snprintf(line, 32, "Deauth:%lu Bcn:%lu", sniff_pktDeauth, sniff_pktBeacon);
    u8g2.drawStr(0, infoY, line);
    
    // Total + controls
    infoY += 8;
    snprintf(line, 32, "Tot:%lu", sniff_pktTotal);
    u8g2.drawStr(0, infoY, line);
    u8g2.drawStr(72, infoY, "[SEL]hop");
    
    u8g2.sendBuffer();
  }
  
  // Handle input: SELECT toggles channel hop on/off
  if (millis() - lastInput > 200) {
    if (digitalRead(BTN_SELECT) == LOW) {
      sniff_channelHop = !sniff_channelHop;
      lastInput = millis();
    }
    // UP/DOWN for manual channel when hop is off
    if (!sniff_channelHop) {
      if (digitalRead(BTN_UP) == LOW) {
        sniff_channel = (sniff_channel % 11) + 1;
        esp_wifi_set_channel(sniff_channel, WIFI_SECOND_CHAN_NONE);
        lastInput = millis();
      }
      if (digitalRead(BTN_DOWN) == LOW) {
        sniff_channel = sniff_channel > 1 ? sniff_channel - 1 : 11;
        esp_wifi_set_channel(sniff_channel, WIFI_SECOND_CHAN_NONE);
        lastInput = millis();
      }
    }
  }
}
