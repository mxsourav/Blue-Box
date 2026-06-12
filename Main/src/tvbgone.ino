#include "WORLD_IR_CODES.h"

// Region constants
#define REGION_NA 0
#define REGION_EU 1

static int tvbRegion = REGION_NA;
static bool tvbTransmitting = false;
static int tvbCurrentCode = 0;
static int tvbTotalCodes = 0;
static unsigned long tvbLastSendTime = 0;
static bool tvbPaused = false;

// Global helper variables for bit-decompression
static uint8_t tvbBitsLeft = 0;
static uint8_t tvbBits = 0;
static uint8_t tvbCodePtr = 0;

uint8_t tvb_read_bits(const IrCode* powerCode, uint8_t count) {
  uint8_t tmp = 0;
  while (count--) {
    if (tvbBitsLeft == 0) {
      tvbBits = powerCode->codes[tvbCodePtr++];
      tvbBitsLeft = 8;
    }
    tmp = (tmp << 1) | ((tvbBits >> --tvbBitsLeft) & 1);
  }
  return tmp;
}

void runTVBGone() {
  static const int num_NAcodes = sizeof(NApowerCodes) / sizeof(NApowerCodes[0]);
  static const int num_EUcodes = sizeof(EUpowerCodes) / sizeof(EUpowerCodes[0]);

  if (!tvbTransmitting) {
    // --- REGION SELECTION UI ---
    if (isButtonPressed(BTN_UP) || isButtonPressed(BTN_DOWN)) {
      tvbRegion = (tvbRegion == REGION_NA) ? REGION_EU : REGION_NA;
    }

    if (isButtonPressed(BTN_SELECT)) {
      tvbTransmitting = true;
      tvbCurrentCode = 0;
      tvbTotalCodes = (tvbRegion == REGION_NA) ? num_NAcodes : num_EUcodes;
      tvbLastSendTime = millis();
      tvbPaused = false;

      pinMode(irsenderpin, OUTPUT);
      digitalWrite(irsenderpin, LOW);
      IrSender.begin(irsenderpin);
    }

    // Draw UI
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.drawStr(12, 16, "TV-B-GONE");
    
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(10, 32, "Select Region:");

    u8g2.setFont(u8g2_font_7x14_tf);
    if (tvbRegion == REGION_NA) {
      u8g2.drawRBox(4, 34, 120, 14, 2);
      u8g2.setDrawColor(0);
      u8g2.drawStr(12, 45, " > NA / ASIA");
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(12, 45, "   NA / ASIA");
    }

    if (tvbRegion == REGION_EU) {
      u8g2.drawRBox(4, 49, 120, 14, 2);
      u8g2.setDrawColor(0);
      u8g2.drawStr(12, 60, " > EU / EUROPE");
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(12, 60, "   EU / EUROPE");
    }
    
    u8g2.sendBuffer();

  } else {
    // --- ACTIVE TRANSMISSION LOOP ---
    if (tvbCurrentCode < tvbTotalCodes) {
      // Get the current power code
      const IrCode* powerCode;
      if (tvbRegion == REGION_NA) {
        powerCode = NApowerCodes[tvbCurrentCode];
      } else {
        powerCode = EUpowerCodes[tvbCurrentCode];
      }

      // Decompress and build raw signal
      uint8_t numpairs = powerCode->numpairs;
      uint8_t bitcompression = powerCode->bitcompression;
      uint16_t freq = cyclesPerPulse(powerCode->timer_val);

      // Reset bit reader
      tvbBitsLeft = 0;
      tvbCodePtr = 0;

      // Build raw data array
      // Cap numpairs to prevent stack overflow from corrupted data
      #define TVB_MAX_PAIRS 128
      if (numpairs > TVB_MAX_PAIRS) numpairs = TVB_MAX_PAIRS;
      uint16_t rawData[TVB_MAX_PAIRS * 2];
      for (uint8_t k = 0; k < numpairs; k++) {
        uint8_t ti = tvb_read_bits(powerCode, bitcompression);
        rawData[k * 2] = powerCode->times[ti * 2];
        rawData[k * 2 + 1] = powerCode->times[ti * 2 + 1];
      }

      // Draw transmitting UI
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(4, 12, "TRANSMITTING...");

      // Animated wave icon
      int waveRadius = (millis() / 150) % 4;
      u8g2.drawCircle(112, 10, 2, U8G2_DRAW_ALL);
      if (waveRadius >= 1) u8g2.drawCircle(112, 10, 4, U8G2_DRAW_ALL);
      if (waveRadius >= 2) u8g2.drawCircle(112, 10, 6, U8G2_DRAW_ALL);
      if (waveRadius >= 3) u8g2.drawCircle(112, 10, 8, U8G2_DRAW_ALL);

      u8g2.drawStr(0, 26, (tvbRegion == REGION_NA) ? "Region: NORTH AMERICA" : "Region: EUROPE / EU");
      
      char codeStats[32];
      snprintf(codeStats, sizeof(codeStats), "Code: %d / %d", tvbCurrentCode + 1, tvbTotalCodes);
      u8g2.drawStr(0, 40, codeStats);

      // Progress bar
      int progressWidth = (tvbCurrentCode * 120) / tvbTotalCodes;
      u8g2.drawFrame(4, 48, 120, 10);
      u8g2.drawBox(6, 50, progressWidth, 6);

      u8g2.sendBuffer();

      // Transmit the raw IR signal
      IrSender.sendRaw(rawData, numpairs * 2, freq);
      digitalWrite(irsenderpin, LOW);

      tvbCurrentCode++;
      delay(205);

    } else {
      // Transmission completed
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_profont17_tr);
      u8g2.drawStr(12, 24, "COMPLETED!");
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(5, 45, "All power codes sent.");
      u8g2.sendBuffer();
      
      delay(2000);
      tvbTransmitting = false;
    }
  }
}

void resetTVBGone() {
  tvbTransmitting = false;
  tvbPaused = false;
  digitalWrite(irsenderpin, LOW);
}

// Helper: convert timer_val to frequency in kHz for IrSender.sendRaw
uint16_t cyclesPerPulse(uint8_t timer_val) {
  return timer_val;  // timer_val is already freq/1000 due to our macro
}
