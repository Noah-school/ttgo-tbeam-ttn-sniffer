/*

  Main module for LoRa Sniffer

*/

#include "configuration.h"
#include "rom/rtc.h"
#include <TinyGPS++.h>
#include <Wire.h>

#include "axp20x.h"
AXP20X_Class axp;
bool pmu_irq = false;

bool ssd1306_found = false;
bool axp192_found = false;

// Forward declarations for sniffer functions
bool lora_setup();
void screen_setup();
void screen_print(const char* text);
void screen_print(String text);
void screen_clear();
void screen_display();
void processLoRaPacket();


// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------

void scanI2Cdevice(void)
{
    byte err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if (err == 0) {
            Serial.print("I2C device found at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.print(addr, HEX);
            Serial.println(" !");
            nDevices++;

            if (addr == SSD1306_ADDRESS) {
                ssd1306_found = true;
                Serial.println("ssd1306 display found");
            }
            if (addr == AXP192_SLAVE_ADDRESS) {
                axp192_found = true;
                Serial.println("axp192 PMU found");
            }
        } else if (err == 4) {
            Serial.print("Unknow error at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.println(addr, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

void axp192Init() {
    if (axp192_found) {
        if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
            Serial.println("AXP192 Begin PASS");
        } else {
            Serial.println("AXP192 Begin FAIL");
        }
        
        axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // LORA radio
        axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // GPS main power (keep on for now)
        axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
        axp.setDCDC1Voltage(3300);
    } else {
        Serial.println("AXP192 not found");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting single-channel LoRa sniffer...");

    Wire.begin(I2C_SDA, I2C_SCL);
    scanI2Cdevice();

    axp192Init();

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    #ifdef LED_PIN
        pinMode(LED_PIN, OUTPUT);
    #endif

    if (ssd1306_found) {
      screen_setup();
    }

    if (!lora_setup()) {
        screen_print("Radio Error!");
        screen_display();
        while(1);
    }
}

void loop() {
    processLoRaPacket();
    delay(100); 
}
