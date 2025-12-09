/*
 * Simplified Screen module for LoRa Sniffer
 */

#include <Wire.h>
#include "SSD1306Wire.h"
#include "fonts.h"

SSD1306Wire *display;
int line = 0;
const int MAX_LINES = 5;

void screen_setup() {
    display = new SSD1306Wire(SSD1306_ADDRESS, I2C_SDA, I2C_SCL);
    if (display && display->init()) {
        display->flipScreenVertically();
        display->setFont(ArialMT_Plain_10);
        display->clear();
        display->drawString(0, 0, "Screen Init OK");
        display->display();
        Serial.println("Screen setup done.");
    } else {
        Serial.println("Screen setup failed.");
        display = nullptr;
    }
}

void screen_clear() {
    if (!display) return;
    display->clear();
    line = 0;
}

void screen_print(String text) {
    if (!display) {
        Serial.println(text);  // Fallback to serial
        return;
    }

    // Simple scrolling text implementation
    if (line >= MAX_LINES) {
        // This is a simplified approach. A real implementation
        // would require a framebuffer/log buffer to scroll properly.
        // For now, we'll just clear and restart.
        display->clear();
        line = 0;
    }
    
    if (display) {
        display->drawString(0, line * 12, text);
    }
    line++;
}

void screen_display() {
    if (!display) return;
    display->display();
}

// Overload for const char*
void screen_print(const char* text) {
  screen_print(String(text));
}