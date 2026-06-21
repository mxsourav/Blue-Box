
  /*

   ██╗  ██╗██╗███████╗███╗   ███╗ ██████╗ ███████╗
   ██║  ██║██║     ██╝████╗ ████║██╔═══██╗██╔════╝
   ███████║██║  ███╗  ██╔████╔██║██║   ██║███████╗
   ██╔══██║██║██╔══╝  ██║╚██╔╝██║██║   ██║╚════██║
   ██║  ██║██║███████╗██║ ╚═╝ ██║╚██████╔╝███████║
   ╚═╝  ╚═╝╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝

              — CREATED BY HIKTRON —

   - HIZMOS IS A FLIPPER ZERO REPLICA 'OPEN SOURCE' -
              
             ......... DEVELOPERS ........
            {YOUSSEF I HIKAL && OMAR KAMEL} 
              
              """ MULTI TOOL DEVICE """

              <FOR>

              [*PENTESTERS & EMBED ENGINEERS 
              HOBBYIST , CYBER SECURITY EXPERTS*]

               #-FEATURES:

               1-WIFI ATTACKS
               2-BLE ATTACKS
               3-BAD USB
               4-NFC
               5-INFRARED
               6-SUB-GHZ
               7-GPIO
               8-APPS
               9-SETTINGS
               10-FILES   

               *********NOTE**********
               ********SCREEN CONFIG*******

               UNCOMMENT THIS LINE DOWN IN CODE IF YOU ARE USING 1.3 OLED
               U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
               UNCOMMENT THIS LINE DOWN IN CODE IF YOU ARE USING 0.96 OR 1.54 OLED
               U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE);
               ********************************************************************

*/
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "animations.h"
#include "WORLD_IR_CODES.h"
#include "mainmenu.h"
#include "dolphinreactions.h"
#include <stdint.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include "logo_hat.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <Update.h>
#include <vector>
#include "USB.h"
#include "USBHIDKeyboard.h"
//#include <BleMouse.h>
//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEScan.h>
#include <WiFi.h>
#include <IRremote.h>
#include <Wire.h>
#define Serial Serial0

#include <esp_wifi.h>
#include "esp_heap_caps.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Override the precompiled esp_brownout_init to do nothing.
// The linker flag -Wl,-zmuldefs in platformio.ini allows this override to replace the system's version.
extern "C" void esp_brownout_init(void) {
    // Do nothing to completely bypass BOD initialization at boot
}
#include "spi_flash_mmap.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "version.h"

// Forward declarations to prevent compiler scope errors
void loading();
//void scanningwifi();
//void deautherAttackLoop();
void runTVBGone();
void resetTVBGone();
uint16_t cyclesPerPulse(uint8_t timer_val);
//String wifi_encryptionType(wifi_auth_mode_t encryption);
void checksysdevices();
void datausage();
void updateTimer();
void drawnosdcard();
void drawHistogram();
void SPECTRUMANALYZER();
void JAMMINGCC1101();
void drawBruteForce();
void drawSettingsMenu();
void handleinfraredmenu();
void handlesubghzmenu();
void handlegpiomenu();
void handlenrftoolsmenu();
void handlewifimenu();
void handleblemenu();
void handleappsmenu();
void handlesettingsmenu();
void handlenfcmenu();
void handlebadusbmenu();
void filesetup();
void filemenu();
void handlesuniversalremotemenu();

USBHIDKeyboard Keyboard;

// ===== Shared WiFi types and globals =====
#define WIFI_MAX_NETWORKS 32

struct WiFiNetwork {
  char ssid[33];
  uint8_t bssid[6];
  int32_t rssi;
  uint8_t channel;
  wifi_auth_mode_t encryption;
};

WiFiNetwork scannedNets[WIFI_MAX_NETWORKS];
int wifi_netCount = 0;
int wifi_selIndex = 0;
int wifi_scrollOffset = 0;
bool wifi_inDetail = false;
bool wifi_inAttackMenu = false;

//BleMouse mouse_ble("hizmos", "hizmos", 100);





/*
struct blescanner_Device {
  String name;
  String address;
  int rssi;
  String manufacturer;
  String deviceType;
};

std::vector<blescanner_Device> blescanner_devices;
int blescanner_selectedIndex = 0;
BLEScan* blescanner_pBLEScan;
*/









//int wifi_selectedIndex = 0;
//int wifi_networkCount = 0;
//bool wifi_showInfo = false;

// ===== WiFi Deauther State =====
// bool deauthMode = false;
// bool deauthActive = false;
// unsigned long deauthFrameCount = 0;
// unsigned long deauthFailedCount = 0; // Tracks failed transmissions
// unsigned long deauthFrameCounterThisSecond = 0;
// unsigned long deauthFps = 0;
// unsigned long lastDeauthSendTime = 0;
// unsigned long lastFpsUpdateTime = 0;
// String deauthTargetSSID = "";
// String deauthTargetMACStr = "";
// uint8_t deauthTargetMAC[6] = {0};
// uint8_t deauthTargetChannel = 1;

/*
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  
 U8G2_R0,
  U8X8_PIN_NONE // Reset pin not in action
 );
*/

class DummyU8G2 {
public:
  void begin() {}
  void beginSimple() {}
  void initDisplay() {}
  void setPowerSave(int) {}
  void clearBuffer() {}
  void sendBuffer() {}
  void setFont(const uint8_t*) {}
  void drawStr(int, int, const char*) {}
  void drawFrame(int, int, int, int) {}
  void drawBox(int, int, int, int) {}
  void setFontMode(int) {}
  void setBitmapMode(int) {}
  void drawXBMP(int, int, int, int, const uint8_t*) {}
  void drawRBox(int, int, int, int, int) {}
  void setDrawColor(int) {}
  void drawPixel(int, int) {}
  void drawRFrame(int, int, int, int, int) {}
  void setCursor(int, int) {}
  template <typename T, typename... Args> void print(T val, Args... args) {}
  void drawHLine(int, int, int) {}
  void drawVLine(int, int, int) {}
  void drawLine(int, int, int, int) {}
  void sendF(const char*, ...) {}
  void drawGlyph(int, int, int) {}
  int getStrWidth(const char*) { return 0; }
  int getStrWidth(String) { return 0; }
  void drawDisc(int, int, int) {}
  void drawCircle(int, int, int, int opt=0) {}
  void printf(const char*, ...) {}
};

DummyU8G2 u8g2;

// ===== Display Invert =====
bool invertUI = false;

void forceDisplayNormal() {
  u8g2.sendF("c", 0xa6);
}

void applyDisplayInvert() {
  u8g2.sendF("c", invertUI ? 0xa7 : 0xa6);
}








#define LED_PIN    48   
#define LED_COUNT  1    

// RMT-free high-precision bitbang driver for WS2812B on ESP32-S3 (GPIO 48)
#include "soc/gpio_reg.h"

// Define RTC memory variables to track successive boots and crashed states
RTC_DATA_ATTR int rtcBootCount = 0;
bool bleInitialized = false;

// Diagnostic LED active states
uint8_t diagR = 0, diagG = 0, diagB = 0;
bool diagFlash = false;
unsigned long diagStartTime = 0;
const unsigned long DIAG_SHOW_DURATION = 10000; // Show diagnostic LED for 10 seconds on boot

void bitbangWS2812(uint8_t pin, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(80); // Reset pulse
  
  uint32_t pinBit;
  uint32_t reg_ts, reg_tc;
  if (pin >= 32) {
    pinBit = 1ULL << (pin - 32);
    reg_ts = GPIO_OUT1_W1TS_REG;
    reg_tc = GPIO_OUT1_W1TC_REG;
  } else {
    pinBit = 1ULL << pin;
    reg_ts = GPIO_OUT_W1TS_REG;
    reg_tc = GPIO_OUT_W1TC_REG;
  }
  
  // Inline assembly cycle counter helper (Xtensa CCOUNT)
  auto get_cycles = []() -> uint32_t {
    uint32_t ccount;
    asm volatile("rsr %0, ccount" : "=r"(ccount));
    return ccount;
  };
  
  portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL(&myMutex);
  
  for (int i = 23; i >= 0; i--) {
    if ((color >> i) & 1) {
      REG_WRITE(reg_ts, pinBit);
      uint32_t start = get_cycles();
      while ((get_cycles() - start) < 168); // 0.7us at 240MHz
      REG_WRITE(reg_tc, pinBit);
      start = get_cycles();
      while ((get_cycles() - start) < 144); // 0.6us at 240MHz
    } else {
      REG_WRITE(reg_ts, pinBit);
      uint32_t start = get_cycles();
      while ((get_cycles() - start) < 84);  // 0.35us at 240MHz
      REG_WRITE(reg_tc, pinBit);
      start = get_cycles();
      while ((get_cycles() - start) < 192); // 0.8us at 240MHz
    }
  }
  
  portEXIT_CRITICAL(&myMutex);
  delayMicroseconds(80); // Reset pulse
}

//  function to set color FOR NEO PIXEL LED (R,G,B) 
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  bitbangWS2812(LED_PIN, r, g, b);
}








#define I2C_SDA 9
#define I2C_SCL 8

// ==== IR Pins ====
#define irsenderpin  15  // Moved from 35 (PSRAM pin)
#define irrecivepin  3   // Moved from 36 (PSRAM pin)

// SD detect pin is disabled to free up GPIO 0 (BOOT) for the external boot button setup.

// ===== NRF24L01 SPI (FSPI) - ACTIVE MODULE =====
#define NRF_SCK   18
#define NRF_MISO  12
#define NRF_MOSI  13

// ===== CC1101 shares SPI with NRF24 (FSPI) - NO MODULE PRESENT =====
#define cc1101_SCK   18
#define cc1101_MISO  12
#define cc1101_MOSI  13


//////////////cc1101(1)////////// (Dummy)
#define CC1101_CS    99
#define CC1101_GDO0  99
#define CC1101_GDO2  99
// ===== CC1101 #2 - NOT PRESENT (dummy pins) =====
#define CC1101_2_CS    99
#define CC1101_2_GDO0  99
#define CC1101_2_GDO2  99




// SD Card via HSPI
#define SD_SCK    14
#define SD_MISO   16
#define SD_MOSI   17
#define SD_CS     46

// ===== NRF24 radio1 - ACTIVE MODULE =====
#define CE1_PIN   21
#define CSN1_PIN  47

// ===== NRF24 radio2 - NOT PRESENT (dummy pins) =====
#define CE2_PIN   99
#define CSN2_PIN   99

// RF24 objects using fspi
SPIClass RADIO_SPI(FSPI);
SPIClass SD_SPI(HSPI);


RF24 radio1(CE1_PIN, CSN1_PIN);
RF24 radio2(CE2_PIN, CSN2_PIN);

// ===== CONTROLLED ISOLATION TEST GLOBALS & FUNCTIONS =====
QueueHandle_t test3Queue = NULL;
TaskHandle_t test3TaskHandle = NULL;
void test3WorkerTask(void* pvParameters) {
  int val;
  while (true) {
    if (xQueueReceive(test3Queue, &val, portMAX_DELAY) == pdTRUE) {
      // Dummy process
    }
  }
}
void IRAM_ATTR emptySnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  // Empty callback to test raw ISR traffic overhead
}



// ==== IR ====
IRrecv irrecv(irrecivepin);
IRsend irsend(irsenderpin);
decode_results results;





// Pins for buttons (reallocated to match physical D-pad + SELECT + BACK wiring)
#define BTN_UP     4
#define BTN_DOWN   11
#define BTN_LEFT   6
#define BTN_RIGHT  10
#define BTN_SELECT 5
#define BTN_BACK   7

extern bool gotoMainMenuFlag;
extern int lastBackClickType;
extern unsigned long lastBackPressTime;
extern int backPressCount;

int custom_digitalRead(uint8_t pin);
#define digitalRead(pin) custom_digitalRead(pin)

// Jamming time per channel (ms)
#define JAM_DURATION 500


const int totalAutoImages = 406;
const int totalManualImages = 11;


int batteryPercent = 87;    // أو من قراءاتك
bool sdOK = true;           // يعتمد على if (SD.begin(...))


///////////////////////////////////////
///////////////////////////////////////
bool inhizmosmenu =0;

int autoImageIndex = 0;
int manualImageIndex = 0;

bool autoMode = true;
unsigned long lastImageChangeTime = 0;
unsigned long lastButtonPressTime = 0;
const unsigned long autoModeTimeout = 120000; //
///////////////////////////////////////////////////////////////
/*







void runLoop(void (*func)()) {
  while (digitalRead(BTN_BACK) == HIGH) {
    func();

   
    if (digitalRead(BTN_SELECT) == LOW) {
      waitForRelease(BTN_SELECT);
    }
  }

  waitForRelease(BTN_BACK);  
}

void waitForRelease(uint8_t pin) {
  while (digitalRead(pin) == LOW);  
}

*/

////////////////////////////////////////////////////////////////////

/////////////////cc1101 vars//////////////

// ============== RF SETTINGS ==============
float currentFreq = 433.92;
float dataRate = 3.79372;  
float deviation = 0.0;    
float rxBW = 325.0;
int powerLevel = 10;


//////////////////////////

///////////////////////////

// Modulation modes
enum ModType { MOD_ASK_OOK = 0, MOD_2FSK = 1, MOD_GFSK = 2, MOD_MSK = 3 };
ModType currentMod = MOD_ASK_OOK; // Default for car remotes

// ============== UI STATE ==============
int mode = -1;  

int menuIndex = 0;
int settingsIndex = 0;

// ============== SD CARD STATE ==============
bool sdCardAvailable = false;
SPIClass hspi(HSPI);  // HSPI for SD card

// ============== BUTTON HANDLING ==============
unsigned long lastButtonPress[4] = {0, 0, 0, 0};
const unsigned long debounceDelay = 150;

bool isButtonPressed(int btn) {
  static int btnMap[] = {BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK};
  int idx = -1;
  for (int i = 0; i < 4; i++) {
    if (btnMap[i] == btn) {
      idx = i;
      break;
    }
  }
  if (idx == -1) return false;
  
  if (!digitalRead(btn)) {
    if (millis() - lastButtonPress[idx] > debounceDelay) {
      lastButtonPress[idx] = millis();
      return true;
    }
  }
  return false;
}

// ============== RAW BUFFER (for pulses) ==============
#define RAW_BUF_MAX 512
#define MAX_SLOTS 5

struct RawPulse {
  unsigned long pulses[RAW_BUF_MAX];
  int length;
  bool valid;
  float frequency;
  unsigned long timestamp;
  char protocol[20];  // NEW: Protocol name
  uint32_t code;      // NEW: Decoded code
  int bitCount;       // NEW: Number of bits
};

RawPulse capturedSlots[MAX_SLOTS];
int currentSlot = 0;
int activeSlot = 0;

// For real-time capture
volatile unsigned long lastEdgeTime = 0;
volatile int pulseIndex = 0;
volatile bool capturing = false;
unsigned long capturePulses[RAW_BUF_MAX];

// Helper to get pulseIndex safely
int getPulseIndex() {
  noInterrupts();
  int idx = pulseIndex;
  interrupts();
  return idx;
}

// ============== PROTOCOL DEFINITIONS ==============
// Most common RF protocols and their characteristics
struct ProtocolDef {
  const char* name;
  int shortPulse;    // Short pulse duration (μs)
  int longPulse;     // Long pulse duration (μs)
  int syncPulse;     // Sync pulse duration (μs)
  int tolerance;     // Tolerance percentage
  int minBits;       // Minimum bits
  int maxBits;       // Maximum bits
};

const ProtocolDef PROTOCOLS[] = {
  {"PT2262",     350,  1050,  10500, 30, 24, 24},  // Most common remote chips
  {"EV1527",     350,  1050,  10500, 30, 24, 24},  // Clone of PT2262
  {"HT12E",      450,  900,   9000,  25, 12, 12},  // Holtek encoder
  {"HT6P20B",    450,  900,   9000,  25, 20, 20},  // Holtek 20-bit
  {"Princeton",  350,  1050,  10500, 30, 24, 24},  // Princeton PT2260/2262
  {"SC5262",     500,  1500,  15000, 30, 24, 24},  // Silan SC5262
  {"Came",       320,  640,   12800, 25, 12, 12},  // Came gate remotes
  {"Nice",       700,  1400,  2800,  25, 12, 24},  // Nice Flo series
  {"Chamberlain",200,  400,   1200,  25, 9,  10},  // Garage door openers
  {"Linear",     500,  1500,  4500,  30, 10, 10},  // Linear MegaCode
  {"KeeLoq",     400,  800,   9600,  25, 66, 66},  // KeeLoq encrypted
  {"Somfy RTS",  640,  1280,  2560,  20, 56, 56},  // Somfy shutters
};

const int PROTOCOL_COUNT = 12;

// ============== STATUS ==============
String statusMsg = "";
unsigned long statusMsgTime = 0;

// ============== HELPER FUNCTIONS ==============
void safeDelay(unsigned long ms) {
  unsigned long end = millis() + ms;
  while (millis() < end) delay(1);
}

void setStatusMsg(String msg) {
  statusMsg = msg;
  statusMsgTime = millis();
  Serial.println(msg);
}

void drawStatusBar() {
  if (millis() - statusMsgTime < 2000 && statusMsg.length() > 0) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(0, 63, statusMsg.c_str());
  }
}

// ============== CC1101 SETUP FOR OOK ==============
void setupOOKMode() {
  Serial.println("Setting up ASK/OOK mode for car remotes...");
  
  ELECHOUSE_cc1101.SetRx();
  ELECHOUSE_cc1101.setMHZ(currentFreq);
  
  // ASK/OOK modulation
  ELECHOUSE_cc1101.setModulation(0); // 0 = ASK/OOK
  
  // Data rate for OOK
  ELECHOUSE_cc1101.setDRate(3.79372);
  
  // No deviation for OOK
  ELECHOUSE_cc1101.setDeviation(0);
  
  // RX Bandwidth
  ELECHOUSE_cc1101.setRxBW(325.0);
  
  // Manchester encoding OFF (car remotes usually don't use it)
  ELECHOUSE_cc1101.setSyncMode(0); // No preamble/sync
  
  // Power
  ELECHOUSE_cc1101.setPA(powerLevel);
  
  Serial.println("OOK mode configured!");
}

void setupFSKMode() {
  Serial.println("Setting up 2-FSK mode...");
  
  ELECHOUSE_cc1101.setModulation(1); // 2-FSK
  ELECHOUSE_cc1101.setDRate(dataRate);
  ELECHOUSE_cc1101.setDeviation(deviation);
  ELECHOUSE_cc1101.setRxBW(rxBW);
  ELECHOUSE_cc1101.setSyncMode(2);
  
  Serial.println("FSK mode configured!");
}

// ============== INTERRUPT FOR PULSE CAPTURE ==============
void IRAM_ATTR pulseISR() {
  unsigned long now = micros();
  if (capturing && pulseIndex < RAW_BUF_MAX) {
    unsigned long duration = now - lastEdgeTime;
    if (duration > 50 && duration < 100000) { // Filter noise
      capturePulses[pulseIndex++] = duration;
    }
    lastEdgeTime = now;
  }
}

enum CCState {
  CC_NOT_INIT,
  CC_OK,
  CC_FAIL
};

enum CCSelect {
  CC_NONE,
  CC_1,
  CC_2
};

CCState cc1State = CC_NOT_INIT;
CCState cc2State = CC_NOT_INIT;

CCSelect activeCC = CC_NONE;

/*
void lazyinit(){
  // Initialize CC1101
  ELECHOUSE_cc1101.setSpiPin(cc1101_SCK, cc1101_MISO, cc1101_MOSI, CC1101_CS);
  ELECHOUSE_cc1101.setGDO(CC1101_GDO0, CC1101_GDO2);
  
  if (ELECHOUSE_cc1101.getCC1101()) {
    Serial.println("CC1101 detected!");
  } else {
    Serial.println("ERROR: CC1101 not found!");
    u8g2.clearBuffer();
    u8g2.drawStr(10, 30, "CC1101 ERROR!");
    u8g2.sendBuffer();
     delay(1000);
  }
  
  ELECHOUSE_cc1101.Init();
  
  // Setup for car remotes (OOK mode)
  setupOOKMode();

  // Setup interrupt for pulse capture
  pinMode(CC1101_GDO0, INPUT);
  attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), pulseISR, CHANGE);

}
*/



bool cc1Inited = false;
bool cc2Inited = false;
void lazyInitCC1101(uint8_t which) {
  // CC1101 is NOT physically connected — all pins are dummy (99)
  // Skip all SPI/GPIO operations to avoid undefined behavior on invalid pins
  Serial.println("CC1101 not present — skipping init");

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 25, "CC1101 Module");
  u8g2.drawStr(10, 40, "Not Connected");
  u8g2.sendBuffer();
  delay(1500);
  return;
}






// ====== Device Control Functions ======
void deactivateSD() {
  digitalWrite(SD_CS, HIGH);
}

void deactivateNRF1() {
  digitalWrite(CSN1_PIN, HIGH);
  digitalWrite(CE1_PIN, LOW);
}

void deactivateNRF2() {
  // radio2 not connected — no GPIO to toggle
}


// ====== Device Control Functions ======
void activateSD() {
  digitalWrite(SD_CS, LOW);
}






bool gotoMainMenuFlag = false;
unsigned long lastBackPressTime = 0;

#define DEBOUNCE_TIME 30  // 30 ms debounce time

static bool debouncedState[48];
static bool lastRawState[48];
static unsigned long lastStateChangeTime[48];
static bool buttonInitDone = false;

// Edge detection: tracks whether a button press has already been consumed
static bool selectConsumed = false;  // true = press already reported, waiting for release
static bool backConsumed = false;

void initButtons() {
  if (buttonInitDone) return;
  uint8_t pins[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT, BTN_BACK};
  for (int i = 0; i < 6; i++) {
    uint8_t pin = pins[i];
    pinMode(pin, INPUT_PULLUP);
    debouncedState[pin] = HIGH; // Active LOW, default HIGH
    lastRawState[pin] = HIGH;
    lastStateChangeTime[pin] = 0;
  }
  selectConsumed = false;
  backConsumed = false;
  buttonInitDone = true;
}

void updatePin(uint8_t pin) {
  bool raw = (digitalRead)(pin); // Raw read
  unsigned long now = millis();
  
  if (raw != lastRawState[pin]) {
    lastStateChangeTime[pin] = now;
    lastRawState[pin] = raw;
  }
  
  if (now - lastStateChangeTime[pin] >= DEBOUNCE_TIME) {
    debouncedState[pin] = raw;
  }
}

void updateBackButton() {
  static bool lastDebouncedBack = HIGH;
  updatePin(BTN_BACK);
  bool currentDebounced = debouncedState[BTN_BACK];
  
  if (currentDebounced == LOW && lastDebouncedBack == HIGH) { // Press transition
    unsigned long now = millis();
    if (now - lastBackPressTime < 350) { // Double click!
      gotoMainMenuFlag = true;
      lastBackPressTime = 0; // Reset
    } else { // First click
      lastBackPressTime = now;
    }
  }
  lastDebouncedBack = currentDebounced;
}

int custom_digitalRead(uint8_t pin) {
  initButtons();
  
  // Update all button pins
  uint8_t pins[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT, BTN_BACK};
  for (int i = 0; i < 6; i++) {
    if (pins[i] == BTN_BACK) {
      updateBackButton();
    } else {
      updatePin(pins[i]);
    }
  }

  if (pin == BTN_BACK) {
    if (gotoMainMenuFlag) {
      return LOW;
    }
    if (debouncedState[BTN_BACK] == HIGH) {
      backConsumed = false;
      return HIGH;
    }
    if (!backConsumed) {
      backConsumed = true;
      return LOW;
    }
    return HIGH;
  }

  // --- EDGE DETECTION for SELECT (OK) button ---
  // Only return LOW once per press. Must release and re-press for next input.
  if (pin == BTN_SELECT) {
    if (debouncedState[BTN_SELECT] == HIGH) {
      // Button is released — reset consumed flag
      selectConsumed = false;
      return HIGH;
    }
    // Button is LOW (pressed)
    if (!selectConsumed) {
      // First time detecting this press — report LOW and consume it
      selectConsumed = true;
      return LOW;
    }
    // Already consumed this press — return HIGH until released
    return HIGH;
  }

  // UP/DOWN/LEFT/RIGHT: level-based (allows hold-to-scroll)
  if (pin == BTN_UP || pin == BTN_DOWN || pin == BTN_LEFT || pin == BTN_RIGHT) {
    return debouncedState[pin];
  }

  // Return HIGH (released) for any invalid or unassigned pins
  if (pin > 48) {
    return HIGH;
  }

  return (digitalRead)(pin); // Default behavior for other pins
}

bool selectPressed() {
  if (digitalRead(BTN_SELECT) == LOW) {
    Serial.println("[BUTTON] SELECT pressed!");
    while (digitalRead(BTN_SELECT) == LOW) {
      delay(10);  // Yield while waiting for button release
    }
    return true;
  }
  return false;
}

void runLoop(void (*func)()) {
  Serial.println("[DBG] runLoop entered");
  Serial.flush();
  unsigned long lastRun = millis();

  while (true) {
    if (gotoMainMenuFlag) {
      break;
    }
   
    if (millis() - lastRun >= 30) {
      func();                 
      lastRun = millis();    
    }

    if (gotoMainMenuFlag) {
      break;
    }

    // Exit with BACK
    if (digitalRead(BTN_BACK) == LOW) {
      Serial.println("[BUTTON] BACK pressed (exit loop)!");
      break;
    }

    // Yield to FreeRTOS scheduler to prevent Task WDT resets
    delay(1);
  }
}












/////////////////////////////////////////////////////////////////////////

void handleButtons() {
  static unsigned long lastInputTime = 0; 
  if (millis() - lastInputTime > 150) {
    if (autoMode) {
      // Idle dolphin state: only BTN_SELECT (OK button) can open the menu
      if (digitalRead(BTN_SELECT) == LOW) {
        Serial.println("[BUTTON] SELECT pressed, opening menu!");
        autoMode = false;
        lastButtonPressTime = millis();
        lastInputTime = millis();
      }
    } else {
      // Menu state: any button press updates lastButtonPressTime to reset screensaver timer
      bool activity = false;
      if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_DOWN) == LOW || 
          digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW || 
          digitalRead(BTN_SELECT) == LOW) {
        activity = true;
      }
      
      if (digitalRead(BTN_BACK) == LOW) {
        Serial.println("[BUTTON] BACK pressed, entering screensaver!");
        autoMode = true;
        activity = true;
      }
      
      if (activity) {
        lastButtonPressTime = millis();
        lastInputTime = millis();
      }
    }
  }
}


void autoModeCheck() {
  if (!autoMode && millis() - lastButtonPressTime > autoModeTimeout) {
    autoMode = true;
  }
}

void displayImage(const uint8_t* image) {
  u8g2.clearBuffer();
  
  u8g2.drawXBMP(0, 0, 128, 64, image);
  u8g2.sendBuffer();
}
void displaymainanim(const uint8_t* image, int batteryPercent, bool sdOK) {


  static const unsigned char image_Background_bits[] U8X8_PROGMEM = {0xfe,0x01,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0xff,0xff,0x0f,0x00,0x00,0x00,0x00,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x7d,0x06,0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xb7,0x55,0x31,0x00,0x00,0x00,0x00,0x81,0xfc,0xff,0xff,0xff,0xff,0xff,0x8f,0x00,0x00,0x00,0xe2,0xff,0xff,0xff,0x7f,0x3d,0x01,0x00,0x00,0x00,0x00,0x00,0x40,0xb6,0xea,0xff,0x04,0x00,0x00,0x00,0x80,0x41,0xfe,0xff,0xff,0xaa,0xfe,0xff,0x3f,0x01,0x00,0x00,0xf9,0xff,0xff,0xff,0xab,0x9f,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xf8,0xff,0x7f,0x02,0x00,0x00,0x00,0x80,0x3e,0xff,0xff,0xff,0xff,0x55,0xfd,0x7f,0xfc,0xff,0xff,0x6c,0xff,0xff,0xff,0xb5,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x80,0x01,0x00,0x00,0x00,0x80,0xc0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x03,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0x00,0x00,0xfe,0xff,0xff,0xff,0x7f};
static const unsigned char image_Battery_bits[] U8X8_PROGMEM = {0xfe,0xff,0x7f,0x00,0x01,0x00,0x80,0x00,0x01,0x00,0x80,0x03,0x01,0x00,0x80,0x02,0x01,0x00,0x80,0x02,0x01,0x00,0x80,0x03,0x01,0x00,0x80,0x00,0xfe,0xff,0x7f,0x00};
static const unsigned char image_SDcardMounted_bits[] U8X8_PROGMEM = {0xff,0x07,0xff,0x04,0xff,0x07,0xff,0x04,0xff,0x07,0xff,0x04,0xff,0x07,0x67,0x00};




  u8g2.clearBuffer();

  // الخلفية (الأنيميشن)
  u8g2.setDrawColor(1);  // أبيض
  u8g2.drawXBMP(0, 0, 128, 64, image);

  
  u8g2.setDrawColor(2);  // XOR mode

  

  // عرض كل شيء
  u8g2.sendBuffer();
}

void runButtonDiagnostic() {
  Serial.println("=== ENTERING BUTTON DIAGNOSTIC ON BOOT (10s) ===");
  initButtons();
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) { // 10 seconds live test
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tr);
    
    // Title & Version
    u8g2.drawStr(2, 7, "DIAGNOSTICS");
    u8g2.drawStr(70, 7, HIZMOS_VERSION);
    u8g2.drawHLine(0, 9, 128);
    
    // Read raw physical state of each pin directly from hardware
    int p4  = (digitalRead)(4);
    int p5  = (digitalRead)(5);
    int p6  = (digitalRead)(6);
    int p7  = (digitalRead)(7);
    int p10 = (digitalRead)(10);
    int p11 = (digitalRead)(11);
    
    char buf[64];
    u8g2.drawStr(2, 18, "BUTTONS:");
    snprintf(buf, sizeof(buf), "UP:G4(%s) DOWN:G11(%s) SEL:G5(%s)", p4 == LOW ? "L" : "H", p11 == LOW ? "L" : "H", p5 == LOW ? "L" : "H");
    u8g2.drawStr(2, 27, buf);
    snprintf(buf, sizeof(buf), "LF:G6(%s) RT:G10(%s) BAK:G7(%s)", p6 == LOW ? "L" : "H", p10 == LOW ? "L" : "H", p7 == LOW ? "L" : "H");
    u8g2.drawStr(2, 36, buf);
    
    u8g2.drawHLine(0, 39, 128);
    u8g2.drawStr(2, 47, "LED FLASH CODES:");
    u8g2.drawStr(2, 55, "GRN:Ok RED:Loop MAG:Crash");
    u8g2.drawStr(2, 63, "YEL:Stk CYA:Brn BLU:Reset");
    
    u8g2.sendBuffer();
    delay(30);
  }
  Serial.println("=== BUTTON DIAGNOSTIC FINISHED ===");
}

void playGlitchAnimation(const uint8_t* baseImage, unsigned long durationMs) {
  unsigned long startTime = millis();
  while (millis() - startTime < durationMs) {
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, baseImage);
    
    // Scanline noise
    for (int i = 0; i < random(0, 4); i++) {
      u8g2.drawHLine(0, random(0, 64), 128);
    }
    
    // Block glitch (20% chance)
    if (random(0, 100) < 20) {
      u8g2.drawBox(random(0, 100), random(0, 58), random(20, 60), random(2, 7));
    }
    
    // XOR invert strip (18% chance)
    if (random(0, 100) < 18) {
      u8g2.setDrawColor(2);
      u8g2.drawBox(0, random(0, 56), 128, random(2, 9));
      u8g2.setDrawColor(1);
    }

    
    u8g2.sendBuffer();
    delay(random(20, 60));
  }
}

void setup() {
  rtcBootCount = 0; // Force reset for active debugging
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
  Serial.begin(115200);
  unsigned long startWait = millis();
  while (!Serial && millis() - startWait < 4000) {
    delay(10);
  }
  Serial.println("--- HIZMOS BOOT START ---");

  // --- DIAGNOSTIC LED CODE ---
  rtcBootCount++;
  esp_reset_reason_t reason = esp_reset_reason();
  diagStartTime = millis();

  if (rtcBootCount > 3) {
    // Boot loop
    diagR = 255; diagG = 0; diagB = 0; // Red
    diagFlash = true;
    Serial.println("DIAGNOSTIC: Boot loop detected (Flashing Red)");
  } else if (reason == ESP_RST_PANIC) {
    // Memory crash / software panic
    diagR = 255; diagG = 0; diagB = 255; // Magenta
    diagFlash = false;
    Serial.println("DIAGNOSTIC: Memory panic/crash detected (Magenta)");
  } else if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT || reason == ESP_RST_WDT) {
    // Watchdog reset / CPU stuck
    diagR = 255; diagG = 255; diagB = 0; // Yellow
    diagFlash = false;
    Serial.println("DIAGNOSTIC: Watchdog reset/CPU stuck detected (Yellow)");
  } else if (reason == ESP_RST_BROWNOUT) {
    // Brownout reset
    diagR = 0; diagG = 255; diagB = 255; // Cyan
    diagFlash = false;
    Serial.println("DIAGNOSTIC: Brownout detected (Cyan)");
  } else if (reason == ESP_RST_POWERON) {
    // Normal boot
    diagR = 0; diagG = 255; diagB = 0; // Green
    diagFlash = false;
    Serial.println("DIAGNOSTIC: Normal Power-on (Green)");
  } else if (reason == ESP_RST_SW) {
    // Software reset
    diagR = 0; diagG = 0; diagB = 255; // Blue
    diagFlash = false;
    Serial.println("DIAGNOSTIC: Software Reset (Blue)");
  } else {
    diagR = 0; diagG = 0; diagB = 255; // Blue
    diagFlash = false;
    Serial.printf("DIAGNOSTIC: Other Reset reason %d (Blue)\n", reason);
  }

  // Set the diagnostic LED color immediately on boot
  setColor(diagR, diagG, diagB);

  Serial.println("Initializing buttons...");
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  Serial.println("Initializing CS/CE pins...");
  pinMode(SD_CS, OUTPUT);
  pinMode(CSN1_PIN, OUTPUT);
  // CSN2/CE2 are dummy pin 99 — skip pinMode
  pinMode(CE1_PIN, OUTPUT);

  Serial.println("Initializing IR...");
  pinMode(irrecivepin, INPUT_PULLUP); // Enable internal pull-up for naked TSOP receiver
  IrReceiver.begin(irrecivepin);
  IrSender.begin(irsenderpin);

  Serial.println("Deactivating devices initially...");
  deactivateSD();
  deactivateNRF1();
  deactivateNRF2();

  Serial.println("Initializing SPI buses (SS=-1)...");
  // Pass -1 as SS pin to prevent SPI library from taking over default SS pins (GPIO 10/11)
  RADIO_SPI.begin(NRF_SCK, NRF_MISO, NRF_MOSI, -1); // FSPI for radios
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, -1);    // HSPI for SD
  Serial.println("I2C and OLED initialization BYPASSED (No screen connected)");

  // Run real-time button diagnostic utility (disabled)
  // runButtonDiagnostic();

  Serial.println("Initializing NeoPixel LED (Disabled)...");
  // pixel.begin();                                         // INITIALIZE NEOPIXEL LED  
  // pixel.setBrightness(80);                              // Optional: reduce brightness
  // pixel.show();                                         // Initialize all pixels to 'off'
  
  Serial.println("Initializing USB Stack...");
  USB.begin();
  Keyboard.begin();

  Serial.println("Drawing start splash screen...");
  drawstartinfo();
  delay(1500);
  
  playGlitchAnimation(logo_hat_bits, 2000);
  delay(500);
  
  Serial.println("Checking system devices (NRF & SD)...");
  checksysdevices();
  delay(1000);
  Serial.println("Deactivating SD card (no card connected)...");
  deactivateSD();
  
  Serial.println("--- HIZMOS BOOT SUCCESSFUL ---");
}

#define HEADLESS_TEST_MODE 1

#ifdef HEADLESS_TEST_MODE
enum HeadlessState {
  STATE_INIT,
  STATE_IDLE_BOOT,
  STATE_SCAN_START,
  STATE_SNIFFER_START,
  STATE_SNIFFER_RUN,
  STATE_SNIFFER_STOP,
  STATE_DEAUTH_START,
  STATE_CYCLE_DONE
};

HeadlessState headlessState = STATE_INIT;
unsigned long stateTimer = 0;

void wifiDoScan();
void snifferStart();
void snifferLoop();
void snifferStop();
void deauthFlood();
extern bool sniff_channelHop;
#endif

void loop() {
  handleButtons();
  autoModeCheck();

  static bool wifiStressActive = false;
  static bool snifferActive = false;

  // === SERIAL COMMAND HANDLER (for headless WiFi testing) ===
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "WIFI_TEST") {
      Serial.println("[WIFI-TEST] === Starting WiFi Scan Test ===");
      Serial.println("[WIFI-TEST] Ensuring WiFi mode is STA...");
      WiFi.mode(WIFI_STA);
      Serial.println("[WIFI-TEST] WiFi mode is STA");
      delay(100);
      Serial.println("[WIFI-TEST] Starting WiFi.scanNetworks()...");
      int n = WiFi.scanNetworks(false, true);
      Serial.printf("[WIFI-TEST] Scan complete. Found %d networks.\n", n);
      for (int i = 0; i < n && i < 10; i++) {
        Serial.printf("[WIFI-TEST]   %d: %s (RSSI: %d, Ch: %d)\n", i+1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
      }
      Serial.println("[WIFI-TEST] Test complete (WiFi kept ON in STA mode).");
      Serial.printf("[WIFI-TEST] Heap after: %lu PSRAM: %lu\n",
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    } else if (cmd == "WIFI_STRESS") {
      wifiStressActive = true;
      Serial.println("[WIFI-STRESS] Loop scan activated. Send 'STOP' to end.");
    } else if (cmd == "SNIFFER_START") {
      snifferActive = true;
      Serial.println("[SNIFFER] Activating packet sniffer...");
      snifferStart();
    } else if (cmd == "SNIFFER_STOP") {
      snifferActive = false;
      Serial.println("[SNIFFER] Deactivating packet sniffer...");
      snifferStop();
    } else if (cmd == "TEST1_START") {
      Serial.println("[TEST1] Starting WiFi...");
      Serial.flush();
      WiFi.mode(WIFI_STA);
      Serial.println("[TEST1] WiFi mode set to STA");
      Serial.flush();
      delay(500);

      Serial.println("[TEST1] Setting max TX power to 8...");
      Serial.flush();
      esp_wifi_set_max_tx_power(8);
      Serial.println("[TEST1] TX power set");
      Serial.flush();
      delay(500);

      Serial.println("[TEST1] Enabling promiscuous mode...");
      Serial.flush();
      esp_wifi_set_promiscuous(false);
      wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL };
      esp_wifi_set_promiscuous_filter(&filter);
      esp_wifi_set_promiscuous(true);
      Serial.println("[TEST1] Promiscuous enabled successfully");
      Serial.flush();
      delay(500);
      Serial.println("[TEST1] Done");
      Serial.flush();
    } else if (cmd == "TEST1_STOP") {
      esp_wifi_set_promiscuous(false);
      WiFi.mode(WIFI_OFF);
      Serial.println("[TEST1] Stopped");
    } else if (cmd == "TEST2_START") {
      Serial.println("[TEST2] Starting WiFi...");
      Serial.flush();
      WiFi.mode(WIFI_STA);
      Serial.println("[TEST2] WiFi mode set to STA");
      Serial.flush();
      delay(500);

      Serial.println("[TEST2] Setting max TX power to 8...");
      Serial.flush();
      esp_wifi_set_max_tx_power(8);
      Serial.println("[TEST2] TX power set");
      Serial.flush();
      delay(500);

      Serial.println("[TEST2] Registering empty callback...");
      Serial.flush();
      esp_wifi_set_promiscuous(false);
      wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL };
      esp_wifi_set_promiscuous_filter(&filter);
      esp_wifi_set_promiscuous_rx_cb(emptySnifferCallback);
      Serial.println("[TEST2] Callback registered");
      Serial.flush();
      delay(500);

      Serial.println("[TEST2] Enabling promiscuous mode...");
      Serial.flush();
      esp_wifi_set_promiscuous(true);
      Serial.println("[TEST2] Promiscuous enabled successfully");
      Serial.flush();
      delay(500);
      Serial.println("[TEST2] Done");
      Serial.flush();
    } else if (cmd == "TEST2_STOP") {
      esp_wifi_set_promiscuous(false);
      esp_wifi_set_promiscuous_rx_cb(NULL);
      WiFi.mode(WIFI_OFF);
      Serial.println("[TEST2] Stopped");
    } else if (cmd == "TEST3_START") {
      Serial.println("[TEST3] Creating queue...");
      test3Queue = xQueueCreate(64, sizeof(int));
      Serial.println("[TEST3] Spawning worker task on Core 0...");
      xTaskCreatePinnedToCore(test3WorkerTask, "test3Task", 4096, NULL, 2, &test3TaskHandle, 0);
      Serial.println("[TEST3] Done");
    } else if (cmd == "TEST3_STOP") {
      if (test3TaskHandle != NULL) {
        vTaskDelete(test3TaskHandle);
        test3TaskHandle = NULL;
      }
      if (test3Queue != NULL) {
        vQueueDelete(test3Queue);
        test3Queue = NULL;
      }
      Serial.println("[TEST3] Stopped");
    } else if (cmd == "TEST4_START") {
      Serial.println("[TEST4] Starting sniffer on fixed CH1...");
      snifferStart();
      sniff_channelHop = false; // Disable channel hopping
      Serial.println("[TEST4] Done");
    } else if (cmd == "TEST4_STOP") {
      snifferStop();
      Serial.println("[TEST4] Stopped");
    } else if (cmd == "STOP") {
      wifiStressActive = false;
      if (snifferActive) {
        snifferActive = false;
        snifferStop();
      }
      // Stop all test modes
      esp_wifi_set_promiscuous(false);
      esp_wifi_set_promiscuous_rx_cb(NULL);
      if (test3TaskHandle != NULL) {
        vTaskDelete(test3TaskHandle);
        test3TaskHandle = NULL;
      }
      if (test3Queue != NULL) {
        vQueueDelete(test3Queue);
        test3Queue = NULL;
      }
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      Serial.println("[ALL TESTS] Stopped.");
    }
  }

  if (wifiStressActive) {
    Serial.println("[WIFI-STRESS] Starting scan...");
    unsigned long start = millis();
    int n = WiFi.scanNetworks(false, true);
    unsigned long duration = millis() - start;
    Serial.printf("[WIFI-STRESS] Scan finished in %lu ms. Found %d networks.\n", duration, n);
    Serial.printf("[WIFI-STRESS] Heap: %lu MaxBlk: %lu PSRAM: %lu\n",
      (unsigned long)esp_get_free_heap_size(),
      (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
      (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    delay(1000); // 1 second delay between scans (no power cycling)
  }

  if (snifferActive) {
    snifferLoop();
  }

  // TEST 3: Push dummy queue data every 10ms
  if (test3Queue != NULL) {
    static unsigned long lastPush = 0;
    if (millis() - lastPush >= 10) {
      lastPush = millis();
      int val = 42;
      xQueueSend(test3Queue, &val, 0);
    }
  }

  // === SYSTEM HEALTH DIAGNOSTIC (every 5s) ===
  static unsigned long lastHealthLog = 0;
  if (millis() - lastHealthLog >= 5000) {
    lastHealthLog = millis();
    Serial.printf("[SYS-HEALTH] Uptime:%lus Heap:%lu MaxBlk:%lu PSRAM:%lu\n",
      millis() / 1000,
      (unsigned long)esp_get_free_heap_size(),
      (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
      (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  }

  // Reset boot count if system runs stably for 10 seconds
  if (millis() > 10000 && rtcBootCount > 0) {
    rtcBootCount = 0;
  }

  // Manage diagnostic LED behavior during the first 10 seconds of boot
  if (millis() - diagStartTime < DIAG_SHOW_DURATION) {
    if (diagFlash) {
      // Flash the LED (e.g. toggle every 250ms)
      if ((millis() / 250) % 2 == 0) {
        setColor(diagR, diagG, diagB);
      } else {
        setColor(0, 0, 0);
      }
    } else {
      setColor(diagR, diagG, diagB);
    }
  } else {
    // Normal operation: turn off LED (set to black)
    setColor(0, 0, 0);
  }
  
  if (autoMode) {
    gotoMainMenuFlag = false; // Reset the escape flag in screensaver
    forceDisplayNormal();
  } else {
    applyDisplayInvert();
  }

  static unsigned long lastRenderTime = 0;
  const unsigned long frameDelay = 30; // 30ms for 33 FPS (33Hz)

  if (millis() - lastRenderTime >= frameDelay) {
    lastRenderTime = millis();

    static bool prevAutoMode = false;
    if (autoMode) {
      bool forceRedraw = (autoMode != prevAutoMode);
      prevAutoMode = autoMode;
      
      if (millis() - lastImageChangeTime > 200 || forceRedraw) {
        if (!forceRedraw) {
          autoImageIndex = (autoImageIndex + 1) % totalAutoImages;
        }
        lastImageChangeTime = millis();
        displaymainanim(autoImages[autoImageIndex], batteryPercent, sdOK);
      }
    } else {
      prevAutoMode = false;
      handlemainmenu(); 
    }
  }
}
