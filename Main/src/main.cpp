#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Change this define to switch stages (1 through 8)
#define INTEGRATION_STAGE 7

// ==========================================
// Stage-specific headers & Pin Definitions
// ==========================================
#if INTEGRATION_STAGE >= 5
#include <Wire.h>
#include <U8g2lib.h>
#define I2C_SDA 9
#define I2C_SCL 8
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
unsigned long lastOledDraw = 0;
#endif

#if INTEGRATION_STAGE >= 6
#include <SPI.h>
#include <SD.h>
#define SD_SCK    14
#define SD_MISO   16
#define SD_MOSI   17
#define SD_CS     46
SPIClass SD_SPI(HSPI);
bool sdInitialized = false;
#endif

#if INTEGRATION_STAGE >= 7
#include <nRF24L01.h>
#include <RF24.h>
#include <IRremote.h>
#define NRF_SCK   18
#define NRF_MISO  12
#define NRF_MOSI  13
#define CE1_PIN   21
#define CSN1_PIN  47
#define IR_RX_PIN 18  // Placeholder for IR receiver
SPIClass RADIO_SPI(FSPI);
RF24 radio(CE1_PIN, CSN1_PIN);
#endif

// ==========================================
// Subsystem Globals & Declarations
// ==========================================
#if INTEGRATION_STAGE >= 1
typedef struct {
  uint8_t payload[64];
  uint16_t length;
  int8_t rssi;
  uint8_t channel;
  uint8_t type;
} SnifferQueueItem;

QueueHandle_t snifferQueue = NULL;
TaskHandle_t snifferTaskHandle = NULL;
volatile uint32_t sniff_pktTotal = 0;
volatile uint32_t sniff_pktDropped = 0;
volatile uint32_t sniff_pktMgmt = 0;
volatile uint32_t sniff_pktData = 0;
volatile uint32_t sniff_pktCtrl = 0;
volatile uint32_t sniff_pktMisc = 0;
volatile uint32_t sniff_pktDeauth = 0;
volatile uint32_t sniff_pktBeacon = 0;
volatile uint32_t sniff_pktProbe = 0;

void snifferWorkerTask(void* pvParameters) {
  SnifferQueueItem item;
  Serial.println("[TASK] snifferWorkerTask started on Core 0.");
  Serial.flush();
  while (true) {
    if (xQueueReceive(snifferQueue, &item, portMAX_DELAY) == pdTRUE) {
      sniff_pktTotal++;
      switch (item.type) {
        case WIFI_PKT_MGMT: {
          sniff_pktMgmt++;
          uint8_t subtype = (item.payload[0] >> 4) & 0x0F;
          if (subtype == 0x0C || subtype == 0x0A) sniff_pktDeauth++;
          else if (subtype == 0x08) sniff_pktBeacon++;
          else if (subtype == 0x04) sniff_pktProbe++;
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
#endif

#if INTEGRATION_STAGE >= 3
volatile uint32_t rawCallbackCount = 0;
void IRAM_ATTR snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  rawCallbackCount++;
#if INTEGRATION_STAGE >= 4
  if (snifferQueue == NULL) return;
  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  SnifferQueueItem item;
  item.length = (pkt->rx_ctrl.sig_len < 64) ? pkt->rx_ctrl.sig_len : 64;
  item.rssi = pkt->rx_ctrl.rssi;
  item.channel = 1; 
  item.type = type;
  memcpy(item.payload, pkt->payload, item.length);
  
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xQueueSendFromISR(snifferQueue, &item, &xHigherPriorityTaskWoken) != pdTRUE) {
    sniff_pktDropped++;
  }
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
#endif
}
#endif

// ==========================================
// Setup and Loop
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n\n==========================================");
  Serial.printf("=== INTEGRATION TEST - STAGE %d BOOTED ===\n", INTEGRATION_STAGE);
  Serial.println("==========================================");
  Serial.flush();
  
  for (int i = 5; i > 0; i--) {
    Serial.printf("Stabilizing rail... Starting in %d seconds...\n", i);
    Serial.flush();
    delay(1000);
  }

#if INTEGRATION_STAGE >= 5
  Serial.println("[INIT] Starting OLED on I2C...");
  Serial.flush();
  Wire.begin(I2C_SDA, I2C_SCL); 
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 20, "BLUE BOX INIT");
  u8g2.setCursor(10, 40);
  u8g2.printf("STAGE: %d", INTEGRATION_STAGE);
  u8g2.sendBuffer();
  Serial.println("[INIT] OLED initialized.");
  Serial.flush();
#endif

#if INTEGRATION_STAGE >= 6
  Serial.println("[INIT] Starting SD Card SPI...");
  Serial.flush();
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
  if (SD.begin(SD_CS, SD_SPI)) {
    sdInitialized = true;
    Serial.println("[INIT] SD Card successfully initialized.");
  } else {
    Serial.println("[ERROR] SD Card initialization failed.");
  }
  Serial.flush();
#endif

#if INTEGRATION_STAGE >= 7
  Serial.println("[INIT] Starting NRF24 SPI...");
  Serial.flush();
  RADIO_SPI.begin(NRF_SCK, NRF_MISO, NRF_MOSI, -1);
  if (radio.begin(&RADIO_SPI)) {
    Serial.println("[INIT] NRF24 successfully initialized.");
  } else {
    Serial.println("[ERROR] NRF24 initialization failed.");
  }
  Serial.flush();

  Serial.println("[INIT] Starting IR Receiver...");
  Serial.flush();
  IrReceiver.begin(IR_RX_PIN);
  Serial.println("[INIT] IR Receiver ready.");
  Serial.flush();
#endif

  // --- RF Subsystem Init ---
  Serial.println("[INIT] Calling WiFi.mode(WIFI_STA)...");
  Serial.flush();
  WiFi.mode(WIFI_STA);
  Serial.println("[INIT] WiFi.mode(WIFI_STA) SUCCESSFUL!");
  Serial.flush();
  delay(500);

#if INTEGRATION_STAGE >= 1
  Serial.println("[INIT] Creating Sniffer Queue...");
  Serial.flush();
  snifferQueue = xQueueCreate(64, sizeof(SnifferQueueItem));
  if (snifferQueue != NULL) {
    Serial.println("[INIT] Sniffer Queue created.");
  }
  Serial.flush();

  Serial.println("[INIT] Spawning Worker Task...");
  Serial.flush();
  xTaskCreatePinnedToCore(
    snifferWorkerTask,
    "snifferWorker",
    4096,
    NULL,
    2,
    &snifferTaskHandle,
    0
  );
  Serial.println("[INIT] Worker task spawned.");
  Serial.flush();
#endif

#if INTEGRATION_STAGE >= 2
  Serial.println("[INIT] Activating promiscuous mode...");
  Serial.flush();
  esp_wifi_set_promiscuous(false);
  wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
  };
  esp_wifi_set_promiscuous_filter(&filter);
#if INTEGRATION_STAGE >= 3
  Serial.println("[INIT] Registering promiscuous callback...");
  Serial.flush();
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);
#endif
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  Serial.println("[INIT] Promiscuous mode enabled.");
  Serial.flush();
#endif
}

void loop() {
  static unsigned long lastLog = 0;
  if (millis() - lastLog >= 2000) {
    lastLog = millis();
    
    UBaseType_t qFill = 0;
    UBaseType_t stackHWM = 0;
#if INTEGRATION_STAGE >= 1
    qFill = (snifferQueue != NULL) ? uxQueueMessagesWaiting(snifferQueue) : 0;
    stackHWM = (snifferTaskHandle != NULL) ? uxTaskGetStackHighWaterMark(snifferTaskHandle) : 0;
#endif

    Serial.printf("[HEALTH] Uptime: %lu s, Free Heap: %lu bytes, QFill: %u, TaskHWM: %u", 
                  millis() / 1000, 
                  esp_get_free_heap_size(), 
                  (unsigned int)qFill, 
                  (unsigned int)stackHWM);
                  
#if INTEGRATION_STAGE >= 3
    Serial.printf(", RawRx: %lu", rawCallbackCount);
#endif

#if INTEGRATION_STAGE >= 4
    Serial.printf(", QRx: %lu, Drops: %lu", sniff_pktTotal, sniff_pktDropped);
#endif

    Serial.println();
    Serial.flush();
  }

#if INTEGRATION_STAGE >= 5
  // Throttled OLED draw (5 FPS, every 200ms)
  if (millis() - lastOledDraw >= 200) {
    lastOledDraw = millis();
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawStr(5, 12, "BLUE BOX HEALTH");
    u8g2.setCursor(5, 24); u8g2.printf("Uptime: %lu s", millis() / 1000);
    u8g2.setCursor(5, 36); u8g2.printf("Heap: %lu B", esp_get_free_heap_size());
#if INTEGRATION_STAGE >= 1
    u8g2.setCursor(5, 48); u8g2.printf("Queue: %u", (unsigned int)uxQueueMessagesWaiting(snifferQueue));
#endif
#if INTEGRATION_STAGE >= 3
    u8g2.setCursor(5, 60); u8g2.printf("Pkts: %lu", rawCallbackCount);
#endif
    u8g2.sendBuffer();
  }
#endif
}
