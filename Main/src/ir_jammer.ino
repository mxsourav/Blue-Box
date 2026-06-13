#include <Arduino.h>
#include <U8g2lib.h>
#include <IRremote.h>

// Global variables for Jammer
static bool jammerActive = false;
static int jammerMode = 0; // 0: BASIC, 1: RANDOM, 2: SWEEP
static int jammerFreq = 0; // 0: 38kHz, 1: 40kHz, 2: 56kHz, 3: SWEEP ALL
static int jammerDensity = 5; // 1 to 10
static int jammerSelectedSetting = 0; // 0: STATUS, 1: MODE, 2: FREQ, 3: DENSITY
static unsigned long jammerSignalCount = 0;
static unsigned long jammerStartTime = 0;
static unsigned long jammerLastSendTime = 0;

// Arrays of frequencies in kHz
static const uint8_t JAM_FREQS[] = {38, 40, 56};
static const int JAM_NUM_FREQS = sizeof(JAM_FREQS) / sizeof(JAM_FREQS[0]);

const char* JAM_MODE_NAMES[] = {"BASIC", "RANDOM", "SWEEP"};
const char* JAM_FREQ_NAMES[] = {"38 kHz", "40 kHz", "56 kHz", "SWEEP ALL"};

void handleIRJammer() {
  unsigned long now = millis();

  // Handle button inputs
  if (isButtonPressed(BTN_UP)) {
    jammerSelectedSetting--;
    if (jammerSelectedSetting < 0) jammerSelectedSetting = 3;
  }
  if (isButtonPressed(BTN_DOWN)) {
    jammerSelectedSetting++;
    if (jammerSelectedSetting > 3) jammerSelectedSetting = 0;
  }

  // Handle LEFT button (decrement)
  if (isButtonPressed(BTN_LEFT)) {
    switch (jammerSelectedSetting) {
      case 0: // STATUS
        jammerActive = !jammerActive;
        if (jammerActive) {
          jammerStartTime = millis();
          jammerSignalCount = 0;
        }
        break;
      case 1: // MODE
        jammerMode--;
        if (jammerMode < 0) jammerMode = 2;
        break;
      case 2: // FREQ
        jammerFreq--;
        if (jammerFreq < 0) jammerFreq = 3;
        break;
      case 3: // DENSITY
        if (jammerDensity > 1) jammerDensity--;
        break;
    }
  }

  // Handle RIGHT or SELECT button (increment/select)
  if (isButtonPressed(BTN_RIGHT) || isButtonPressed(BTN_SELECT)) {
    switch (jammerSelectedSetting) {
      case 0: // STATUS
        jammerActive = !jammerActive;
        if (jammerActive) {
          jammerStartTime = millis();
          jammerSignalCount = 0;
        }
        break;
      case 1: // MODE
        jammerMode++;
        if (jammerMode > 2) jammerMode = 0;
        break;
      case 2: // FREQ
        jammerFreq++;
        if (jammerFreq > 3) jammerFreq = 0;
        break;
      case 3: // DENSITY
        if (jammerDensity < 10) jammerDensity++;
        break;
    }
  }

  // Determine active frequency
  uint8_t freq = 38;
  if (jammerFreq < 3) {
    freq = JAM_FREQS[jammerFreq];
  } else {
    // Sweep all: cycle through 38, 40, 56 kHz based on time
    freq = JAM_FREQS[(millis() / 150) % 3];
  }

  // If jammer is active, send signals based on density
  unsigned long interval = (11 - jammerDensity) * 15;
  if (jammerActive && (now - jammerLastSendTime >= interval)) {
    jammerLastSendTime = now;

    if (jammerMode == 0) { // BASIC
      // Create a raw pattern of alternating mark/space
      // 20 elements, alternating 12us mark / 12us space
      uint16_t basicPattern[20];
      for (int i = 0; i < 20; i += 2) {
        basicPattern[i] = 12;      // Mark
        basicPattern[i+1] = 12;    // Space
      }
      
      // Perform direct pin pulses for raw impact
      for (int b = 0; b < jammerDensity * 5; b++) {
        digitalWrite(irsenderpin, HIGH);
        delayMicroseconds(12);
        digitalWrite(irsenderpin, LOW);
        delayMicroseconds(12);
      }

      // Also send raw block via library
      irsend.sendRaw(basicPattern, 20, freq);
      
    } else if (jammerMode == 1) { // RANDOM
      // Generate randomized pattern
      uint16_t randomPattern[30];
      for (int i = 0; i < 30; i++) {
        randomPattern[i] = random(5, 500); // random pulse width
      }
      
      for (int b = 0; b < jammerDensity; b++) {
        irsend.sendRaw(randomPattern, 30, freq);
      }
      
    } else if (jammerMode == 2) { // SWEEP
      // Dynamic sweep: shift timings from 8us to 60us
      static int sweepVal = 8;
      static int sweepDir = 2;
      
      sweepVal += sweepDir;
      if (sweepVal > 60 || sweepVal < 8) {
        sweepDir = -sweepDir;
        sweepVal = constrain(sweepVal, 8, 60);
      }
      
      uint16_t sweepPattern[20];
      for (int i = 0; i < 20; i += 2) {
        sweepPattern[i] = sweepVal;
        sweepPattern[i+1] = sweepVal;
      }
      
      for (int b = 0; b < jammerDensity * 3; b++) {
        digitalWrite(irsenderpin, HIGH);
        delayMicroseconds(sweepVal);
        digitalWrite(irsenderpin, LOW);
        delayMicroseconds(sweepVal);
      }
      
      irsend.sendRaw(sweepPattern, 20, freq);
    }

    jammerSignalCount++;
  }

  // Draw the User Interface
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  
  u8g2.drawStr(5, 12, "IR JAMMER");
  
  if (jammerActive) {
    if ((millis() / 500) % 2 == 0) {
      u8g2.drawStr(90, 12, "TX...");
    }
  } else {
    u8g2.drawStr(90, 12, "IDLE");
  }

  u8g2.drawHLine(0, 15, 128);

  // Render the adjustable settings
  int ySpacing = 11;
  int startY = 28;

  for (int i = 0; i < 4; i++) {
    int y = startY + (i * ySpacing);
    
    // Draw highlight background if selected
    if (i == jammerSelectedSetting) {
      u8g2.drawRBox(4, y - 10, 120, 12, 2);
      u8g2.setDrawColor(0); // Inverted text
    }

    switch (i) {
      case 0:
        u8g2.drawStr(8, y, "STATUS:");
        u8g2.drawStr(70, y, jammerActive ? "ACTIVE" : "PAUSED");
        break;
      case 1:
        u8g2.drawStr(8, y, "MODE:");
        u8g2.drawStr(70, y, JAM_MODE_NAMES[jammerMode]);
        break;
      case 2:
        u8g2.drawStr(8, y, "FREQ:");
        u8g2.drawStr(70, y, JAM_FREQ_NAMES[jammerFreq]);
        break;
      case 3:
        u8g2.drawStr(8, y, "DENSITY:");
        char densStr[5];
        snprintf(densStr, sizeof(densStr), "%d", jammerDensity);
        u8g2.drawStr(70, y, densStr);
        u8g2.drawFrame(90, y - 8, 30, 7);
        u8g2.drawBox(90, y - 8, jammerDensity * 3, 7);
        break;
    }

    u8g2.setDrawColor(1); // Restore color
  }

  // Draw some stats at the bottom
  u8g2.setFont(u8g2_font_5x8_tr);
  char stats[40];
  if (jammerActive) {
    unsigned long activeSec = (now - jammerStartTime) / 1000;
    snprintf(stats, sizeof(stats), "Jams: %lu   Time: %02lu:%02lu", jammerSignalCount, activeSec / 60, activeSec % 60);
  } else {
    snprintf(stats, sizeof(stats), "Jams: %lu   Time: 00:00", jammerSignalCount);
  }
  u8g2.drawStr(8, 62, stats);

  u8g2.sendBuffer();
}
