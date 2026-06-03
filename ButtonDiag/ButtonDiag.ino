#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE);

#define I2C_SDA 9
#define I2C_SCL 8

const uint8_t pins[] = {4, 5, 6, 7, 10, 11};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- STANDALONE BUTTON DIAGNOSTIC BOOT ---");

  for (int i = 0; i < 6; i++) {
    pinMode(pins[i], INPUT_PULLUP);
  }

  Wire.begin(I2C_SDA, I2C_SCL);
  u8g2.begin();
}

void loop() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr); // Large bold header
  u8g2.drawStr(5, 18, "KEY TEST");
  
  u8g2.setFont(u8g2_font_7x14_tf); // High-readability clean font
  
  int p4  = digitalRead(4);
  int p5  = digitalRead(5);
  int p6  = digitalRead(6);
  int p7  = digitalRead(7);
  int p10 = digitalRead(10);
  int p11 = digitalRead(11);
  
  char buf[32];
  sprintf(buf, "G4: %s   G7: %s", p4 == LOW ? "L" : "H", p7 == LOW ? "L" : "H");
  u8g2.drawStr(5, 34, buf);
  
  sprintf(buf, "G5: %s   G10: %s", p5 == LOW ? "L" : "H", p10 == LOW ? "L" : "H");
  u8g2.drawStr(5, 48, buf);
  
  sprintf(buf, "G6: %s   G11: %s", p6 == LOW ? "L" : "H", p11 == LOW ? "L" : "H");
  u8g2.drawStr(5, 62, buf);
  
  u8g2.sendBuffer();
  delay(50);
}
