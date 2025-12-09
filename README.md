## TTGO T-Beam LoRa Sniffer

A LoRa packet sniffer for the [TTGO T-Beam](https://github.com/LilyGO/TTGO-T-Beam) development platform that captures and displays LoRaWAN packets over a single channel.

This project uses the SSD1306 I2C OLED display and the RFM95 transceiver to monitor LoRa traffic on your configured frequency band (US 915 MHz or EU 868 MHz by default).

### Origin

This is a modified version of [hermans-casa/ttgo-tbeam-ttn-teller](https://github.com/hermans-casa/ttgo-tbeam-ttn-teller), adapted to function as a LoRa sniffer for packet monitoring and analysis instead of a TTN tracker node. Unnecessary tracker and credential management code has been removed to create a lightweight, single-purpose sniffer application.

NOTE: There are 2 versions of the TTGO T-BEAM: Rev0 and Rev1. The GPS module on Rev1 is connected to different pins than Rev0. This code has been tested on both versions.

### Setup

The preferred method to install this library is via [PlatformIO](https://platformio.org/install).

1. Follow the directions at [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) to install the board to the Arduino IDE and use board 'T-Beam'.

2. Install the Arduino IDE libraries:

   * [LoRa](https://github.com/sandeepmistry/arduino-LoRa) (LoRa communication)
   * [mikalhart/TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) (optional, for GPS support)
   * [ThingPulse/esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306) (OLED display support)
   * [lewisxhe/AXP202X_Library](https://github.com/lewisxhe/AXP202X_Library) (power management, Rev1 only)

3. Edit `main/configuration.h` and select your correct board revision (T_BEAM_V07 or T_BEAM_V10).

4. Configure the LoRa frequency band in the build flags of `platformio.ini` by adjusting the `LORA_CHANNEL_INDEX` (0-7 for different channels).

5. Flash the firmware to your TTGO T-Beam using PlatformIO:
   ```bash
   platformio run --target upload
   ```

6. Monitor serial output to see captured LoRa packets:
   ```bash
   platformio device monitor
   ```

The device will display captured LoRa packets on the OLED screen and output detailed packet information to the serial console.
