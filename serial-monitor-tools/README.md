# Blue Box Serial Monitor Tools

This folder contains serial monitor tools for controlling and watching Blue Box during development and testing.

## Files

- `index.html` - browser-based Web Serial panel with command input and virtual D-pad buttons.
- `serial_panel.py` - Python desktop serial monitor with command input and virtual D-pad buttons.

## Virtual Controls

The serial monitor tools provide virtual buttons for:

- Up
- Down
- Left
- Right
- Select
- Back

They send serial button commands to the firmware, making it possible to navigate menus without pressing the physical buttons.

## USB Connection

Connect the USB cable to the ESP32-S3 COM Type-C port.

Do not connect through the native USB-C port when using these serial monitor tools. Use the COM/UART Type-C port so the serial monitor can open the correct COM device.
