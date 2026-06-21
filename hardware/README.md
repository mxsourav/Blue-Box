# Hardware

Hardware notes, pin planning, and board documentation for the Blue Box build.

## Core Parts

- ESP32-S3 N16R8 development board
- SH1106 1.3 inch OLED display
- Momentary push buttons for menu control
- nRF24L01 module
- CC1101 sub-GHz module
- IR transmitter and receiver hardware
- MicroSD card module
- RGB status LED
- 1000uF 25V electrolytic capacitor
- 104 ceramic capacitor

## Notes

- Keep the 1000uF 25V capacitor close to the main power input rail for bulk filtering.
- Use the 104 ceramic capacitor near module power pins for local decoupling.
