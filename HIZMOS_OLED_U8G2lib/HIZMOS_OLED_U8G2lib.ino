
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
#include <BleMouse.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <IRremote.h>
#include <Wire.h>


#include <esp_wifi.h>
#include "esp_heap_caps.h"
#include "spi_flash_mmap.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "version.h"

// --- NEW SNIFFER ARCHITECTURE ---
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define SNIFFER_QUEUE_SIZE 100
QueueHandle_t snifferPacketQueue;

struct SnifferPacket {
  wifi_promiscuous_pkt_type_t type;
  uint16_t length;
};

extern void snifferWorkerTask(void *pvParameters);
// --------------------------------




// Forward declarations to prevent compiler scope errors
void loading();
void scanningwifi();
void deautherAttackLoop();
void runTVBGone();
void resetTVBGone();
uint16_t cyclesPerPulse(uint8_t timer_val);
String wifi_encryptionType(wifi_auth_mode_t encryption);
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

BleMouse mouse_ble("hizmos", "hizmos", 100);





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









int wifi_selectedIndex = 0;
int wifi_networkCount = 0;
bool wifi_showInfo = false;

// ===== WiFi Deauther & Beacon State =====
bool deauthMode = false;
bool beaconMode = false;
bool deauthActive = false;
bool beaconActive = false;
unsigned long txAttemptCount = 0;
unsigned long txSuccessCount = 0;
unsigned long txFailCount = 0;
unsigned long txSuccessCounterThisSecond = 0;
unsigned long txFps = 0;
unsigned long lastAttackSendTime = 0;
unsigned long lastFpsUpdateTime = 0;
String attackTargetSSID = "";
String attackTargetMACStr = "";
uint8_t attackTargetMAC[6] = {0};
uint8_t attackTargetChannel = 1;

// ===== Beacon Spam State & Telemetry =====
QueueHandle_t beaconSpamQueue;
TaskHandle_t beaconSpamTaskHandle = NULL;
bool beaconSpamActive = false;
unsigned long bsAttemptCount = 0;
unsigned long bsSuccessCount = 0;
unsigned long bsFailCount = 0;
unsigned long bsSuccessCounterThisSecond = 0;
unsigned long bsFps = 0;
unsigned long lastBsFpsUpdateTime = 0;
uint16_t bsSequence = 0;
uint8_t bsCurrentChannel = 1;
uint8_t bsMode = 0; 
uint8_t bsAPCount = 50;

struct AttackCommand {
  uint8_t action; // 1 = Start, 0 = Stop
};

extern void beaconSpamWorkerTask(void *pvParameters);

/*
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  
 U8G2_R0,
  U8X8_PIN_NONE // Reset pin not in action
 );
*/

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE);

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

#define SD_DETECT_PIN 0

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


// ===== SAFE GPIO & VIRTUAL INPUT LAYER =====
uint8_t virtualBtnState[256];

bool isValidGPIO(int pin) {
    if (pin < 0 || pin > 48 || pin == 99) return false;
    return true;
}

void safePinMode(int pin, uint8_t mode) {
    if (isValidGPIO(pin)) (pinMode)(pin, mode);
}

void safeDigitalWrite(int pin, uint8_t val) {
    if (isValidGPIO(pin)) (digitalWrite)(pin, val);
}

int custom_digitalRead(uint8_t pin);
#define pinMode(pin, mode) safePinMode(pin, mode)
#define digitalWrite(pin, val) safeDigitalWrite(pin, val)
#define digitalRead(pin) custom_digitalRead(pin)
// ===========================================

// Jamming time per channel (ms)
#define JAM_DURATION 500


const int totalAutoImages = 406;
const int totalManualImages = 11;


int batteryPercent = 87;    // أو من قراءاتك
bool sdOK = true;           // يعتمد على if (false)


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
  // ELECHOUSE_cc1101.setSpiPin(cc1101_SCK, cc1101_MISO, cc1101_MOSI, CC1101_CS);
  // ELECHOUSE_cc1101.setGDO(CC1101_GDO0, CC1101_GDO2);
  
  if (ELECHOUSE_cc1101.getCC1101()) {
    Serial.println("CC1101 detected!");
  } else {
    Serial.println("ERROR: CC1101 not found!");
    u8g2.clearBuffer();
    u8g2.drawStr(10, 30, "CC1101 ERROR!");
    u8g2.sendBuffer();
     delay(1000);
  }
  
  // ELECHOUSE_cc1101.Init();
  
  // Setup for car remotes (OOK mode)
  setupOOKMode();

  // Setup interrupt for pulse capture
  if (CC1101_GDO0 != 99) {
    pinMode(CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), pulseISR, CHANGE);
  }

}
*/



bool cc1Inited = false;
bool cc2Inited = false;
void lazyInitCC1101(uint8_t which) {
  struct CCModule {
    uint8_t cs;
    uint8_t gdo0;
    uint8_t gdo2;
    bool *initedFlag;
    const char* name;
  };

  CCModule modules[2] = {
    {CC1101_CS, CC1101_GDO0, CC1101_GDO2, &cc1Inited, "CC1101 #1"},
    {CC1101_2_CS, CC1101_2_GDO0, CC1101_2_GDO2, &cc2Inited, "CC1101 #2"}
  };

  // أولاً، نفك أي interrupt قديم
  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
  detachInterrupt(digitalPinToInterrupt(CC1101_2_GDO0));

  // عمل init لكل موديول لو مش متعمل قبل كده
  for (int i = 0; i < 2; i++) {
    if (!*(modules[i].initedFlag)) {
      // ELECHOUSE_cc1101.setSpiPin(cc1101_SCK, cc1101_MISO, cc1101_MOSI, modules[i].cs);
      // ELECHOUSE_cc1101.setGDO(modules[i].gdo0, modules[i].gdo2);

      if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.print(modules[i].name);
        Serial.println(" NOT FOUND");
        *(modules[i].initedFlag) = false;
      } else {
        // ELECHOUSE_cc1101.Init();
        setupOOKMode();
        pinMode(modules[i].gdo0, INPUT);
        *(modules[i].initedFlag) = true;
      }
    }
  }

  // بعد الـ init، نختار الـ active module
  if (which == 1 || which == 2) {
    CCModule &selected = modules[which - 1];
    // ELECHOUSE_cc1101.setSpiPin(cc1101_SCK, cc1101_MISO, cc1101_MOSI, selected.cs);
    // ELECHOUSE_cc1101.setGDO(selected.gdo0, selected.gdo2);
    attachInterrupt(digitalPinToInterrupt(selected.gdo0), pulseISR, CHANGE);
  } else {
    Serial.println("Invalid selection");
  }

 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  for (int i = 0; i < 2; i++) {
    const char* status = (*(modules[i].initedFlag)) ? "INIT OK" : "NOT FOUND";
    if ((i + 1) == which) {
      u8g2.drawStr(0, 15 * (i+1), modules[i].name);
      u8g2.drawStr(80, 15 * (i+1), status);
      u8g2.drawStr(100, 15 * (i+1), "<SELECTED>");
    } else {
      u8g2.drawStr(0, 15 * (i+1), modules[i].name);
      u8g2.drawStr(80, 15 * (i+1), status);
    }
  }
  u8g2.sendBuffer();

  Serial.print("CC1101 ACTIVE: ");
  Serial.println(which);
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
  digitalWrite(CSN2_PIN, HIGH);
  digitalWrite(CE2_PIN, LOW);
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
  backConsumed = false;
  buttonInitDone = true;
}

void updatePin(uint8_t pin) {
  bool raw = HIGH;
  bool isVirtual = false;
  if (pin >= 0 && pin < 256 && virtualBtnState[pin] == LOW) {
      raw = LOW;
      isVirtual = true;
  } else if (isValidGPIO(pin)) {
      raw = (digitalRead)(pin); // Raw physical read
  }
  unsigned long now = millis();
  
  if (raw != lastRawState[pin]) {
    lastStateChangeTime[pin] = now;
    lastRawState[pin] = raw;
  }
  
  if (isVirtual || now - lastStateChangeTime[pin] >= DEBOUNCE_TIME) {
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

  // --- EDGE DETECTION for D-pad and SELECT buttons ---
  // Only return LOW once per press. Must release and re-press for next input.
  if (pin == BTN_UP || pin == BTN_DOWN || pin == BTN_LEFT || pin == BTN_RIGHT || pin == BTN_SELECT) {
    static bool edgeConsumed[256] = {false};
    if (debouncedState[pin] == HIGH) {
      // Button is released — reset consumed flag
      edgeConsumed[pin] = false;
      return HIGH;
    }
    // Button is LOW (pressed)
    if (!edgeConsumed[pin]) {
      // First time detecting this press — report LOW and consume it
      edgeConsumed[pin] = true;
      return LOW;
    }
    // Already consumed this press — return HIGH until released
    return HIGH;
  }

  if (pin >= 0 && pin < 256 && virtualBtnState[pin] == LOW) return LOW;
  if (isValidGPIO(pin)) return (digitalRead)(pin);
  return HIGH;
}

bool selectPressed() {
  if (digitalRead(BTN_SELECT) == LOW) {
    Serial.println("[BUTTON] SELECT pressed!");
    while (digitalRead(BTN_SELECT) == LOW);  
    return true;
  }
  return false;
}

void runLoop(void (*func)()) {
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
    sprintf(buf, "UP:G4(%s) DOWN:G11(%s) SEL:G5(%s)", p4 == LOW ? "L" : "H", p11 == LOW ? "L" : "H", p5 == LOW ? "L" : "H");
    u8g2.drawStr(2, 27, buf);
    sprintf(buf, "LF:G6(%s) RT:G10(%s) BAK:G7(%s)", p6 == LOW ? "L" : "H", p10 == LOW ? "L" : "H", p7 == LOW ? "L" : "H");
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

extern "C" int ieee80211_raw_frame_sanity_check(int32_t, int32_t, int32_t);

void setup() {

  for (int i=0; i<256; i++) virtualBtnState[i] = HIGH;

  Serial.begin(115200);
  delay(1000);
  Serial.println("--- HIZMOS BOOT START ---");

  // --- INDIA REGION RF COMPLIANCE ---
  Serial.println("Configuring India WiFi Region (IN, Channels 1-13)...");
  WiFi.mode(WIFI_STA);
  esp_wifi_set_country_code("IN", true);
  wifi_country_t country_config = {
    .cc = "IN",
    .schan = 1,
    .nchan = 13,
    .max_tx_power = 20, // 20 dBm (100mW) limit
    .policy = WIFI_COUNTRY_POLICY_MANUAL
  };
  esp_wifi_set_country(&country_config);
  
  // --- VERIFY SANITY CHECK OVERRIDE LINKAGE ---
  int overrideResult = ieee80211_raw_frame_sanity_check(0, 0, 0);
  if (overrideResult == 0) {
    Serial.println("[BOOT] Sanity check override LINKED and ACTIVE (returns 0 = PASS)");
  } else {
    Serial.printf("[BOOT] WARNING: Override returns %d (expected 0)!\n", overrideResult);
  }
  // --------------------------------
  
  WiFi.mode(WIFI_OFF);
  // --------------------------------

  // --- NEW SNIFFER ARCHITECTURE ---
  snifferPacketQueue = xQueueCreate(SNIFFER_QUEUE_SIZE, sizeof(SnifferPacket));
  if (snifferPacketQueue == NULL) {
    Serial.println("Failed to create sniffer queue!");
  }

  TaskHandle_t snifferWorkerTaskHandle = NULL;
  xTaskCreatePinnedToCore(
    snifferWorkerTask,
    "SnifferWorker",
    4096,
    NULL,
    1,
    &snifferWorkerTaskHandle,
    0
  );

  Serial.println("Creating Beacon Spam Queue & Worker Task on Core 0...");
  beaconSpamQueue = xQueueCreate(5, sizeof(AttackCommand));
  if (beaconSpamQueue == NULL) {
    Serial.println("Failed to create beacon spam queue!");
  }
  xTaskCreatePinnedToCore(
    beaconSpamWorkerTask,
    "BeaconSpam",
    4096,
    NULL,
    1,
    &beaconSpamTaskHandle,
    0
  );
  // --------------------------------

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
  pinMode(SD_DETECT_PIN, INPUT_PULLUP);

  Serial.println("Initializing CS/CE pins...");
  pinMode(SD_CS, OUTPUT);
  if (CSN1_PIN != 99) pinMode(CSN1_PIN, OUTPUT);
  if (CSN2_PIN != 99) pinMode(CSN2_PIN, OUTPUT);
  if (CE1_PIN != 99) pinMode(CE1_PIN, OUTPUT);
  if (CE2_PIN != 99) pinMode(CE2_PIN, OUTPUT);

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
  
  Serial.println("Initializing I2C Wire (SDA=8, SCL=9)...");
  Wire.begin(I2C_SDA, I2C_SCL);  // Explicitly set I2C pins: SDA=GPIO8, SCL=GPIO9
  
  Serial.println("Initializing OLED Display...");
  u8g2.begin();

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
  
  Serial.println("Activating SD card...");
  activateSD();
  
  Serial.println("--- HIZMOS BOOT SUCCESSFUL ---");
}

void loop() {
  handleButtons();
  autoModeCheck();

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

  // --- Serial Command Processing ---
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      if (cmd == "start beacon") {
        Serial.println(">>> SERIAL COMMAND RECEIVED: Starting Beacon Menu <<<");
        deauthActive = false; beaconActive = false; deauthMode = false; beaconMode = true; 
        wifi_selectedIndex = 0; wifi_showInfo = false; autoMode = false;
        runLoop(scanningwifi);
      }
      else if (cmd == "start wifi") {
        Serial.println(">>> SERIAL COMMAND RECEIVED: Starting WiFi Menu <<<");
        deauthActive = false; beaconActive = false; deauthMode = true; beaconMode = false; 
        wifi_selectedIndex = 0; wifi_showInfo = false; autoMode = false;
        runLoop(handlewifimenu);
      }
      else if (cmd.startsWith("btn ")) {
        int pin = -1;
        bool isRelease = false;
        String action = cmd.substring(4);
        
        if (action.startsWith("release ")) {
            isRelease = true;
            action = action.substring(8);
        }
        
        if (action == "up") pin = BTN_UP;
        else if (action == "down") pin = BTN_DOWN;
        else if (action == "left") pin = BTN_LEFT;
        else if (action == "right") pin = BTN_RIGHT;
        else if (action == "select") pin = BTN_SELECT;
        else if (action == "back") pin = BTN_BACK;
        
        if (pin != -1) {
            virtualBtnState[pin] = isRelease ? HIGH : LOW;
            Serial.print("[VIRTUAL INPUT] ");
            Serial.print(action);
            Serial.println(isRelease ? " RELEASED" : " PRESSED");
        }
      }
    }
  }
}
