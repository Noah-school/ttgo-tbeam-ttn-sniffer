/*
 * LoRa Sniffer module
 * 
 * Implements a single-channel LoRa sniffer for the TTGO T-Beam.
 * It displays received packet information on the OLED screen and
 * outputs structured data to the Serial port.
 */

#include <LoRa.h>

// LORA_CHANNEL_INDEX is defined via build_flags in platformio.ini

// Define the EU868 frequencies for TTN
uint32_t channels[] = {
  868100000, // Index 0
  868300000, // Index 1
  868500000, // Index 2
  867100000, // Index 3
  867300000, // Index 4
  867500000, // Index 5
  867700000, // Index 6
  867900000  // Index 7
};
uint8_t sf = 7; // Default spreading factor

// Forward declarations from screen.ino
void screen_print(const char* text);
void screen_print(String text);
void screen_clear();
void screen_display();

// Function to escape strings for JSON
String jsonEscape(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\b", "\\b");
  s.replace("\f", "\\f");
  s.replace("\n", "\\n");
  s.replace("\r", "\\r");
  s.replace("\t", "\\t");
  return s;
}

// Flag to indicate a packet was received
volatile bool packet_received = false;
volatile int last_packet_size = 0;

// Function to be called when a packet is received (ISR context - keep it simple!)
void onLoRaPacket(int packetSize) {
  if (packetSize == 0) return;
  last_packet_size = packetSize;
  packet_received = true;
}

// Function to process the received packet (called from main loop)
void processLoRaPacket() {
  if (!packet_received) return;
  packet_received = false;
  
  // Read packet payload as raw bytes so we can show ASCII and hex
  uint8_t bytes[256];
  int len = 0;
  while (LoRa.available() && len < (int)sizeof(bytes)) {
    int b = LoRa.read();
    if (b < 0) break;
    bytes[len++] = static_cast<uint8_t>(b);
  }

  String payload = "";       // ASCII-ish view
  String hexPayload = "";    // Raw hex view
  for (int i = 0; i < len; i++) {
    payload += (char)bytes[i];

    char buf[4];
    snprintf(buf, sizeof(buf), "%02X", bytes[i]);
    if (i > 0) hexPayload += " ";
    hexPayload += buf;
  }
  
  float frequency = channels[LORA_CHANNEL_INDEX] / 1000000.0;
  float rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  // Display packet information on the screen
  screen_clear();
  
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "CH: %.1fMHz", frequency);
  screen_print(buffer);
  
  snprintf(buffer, sizeof(buffer), "RSSI: %.0f dBm", rssi);
  screen_print(buffer);

  snprintf(buffer, sizeof(buffer), "SNR: %.2f dB", snr);
  screen_print(buffer);

  screen_print("HEX payload:");
  screen_print(hexPayload.length() ? hexPayload : String("<empty>"));

  screen_display();

  // Print structured JSON to Serial for the Python script
  Serial.print("{\"channel\": ");
  Serial.print(frequency, 1);
  Serial.print(", \"rssi\": ");
  Serial.print(rssi, 0);
  Serial.print(", \"snr\": ");
  Serial.print(snr, 2);
  Serial.print(", \"payload\": \"");
  Serial.print(jsonEscape(payload));
  Serial.print("\", \"payload_hex\": \"");
  Serial.print(hexPayload);
  Serial.print("\", \"payload_len\": ");
  Serial.print(len);
  Serial.println("}");
}

// Function to set up the LoRa module
bool lora_setup() {
  Serial.println("Setting up LoRa...");
  
  // LoRa pin configuration for TTGO T-Beam
  LoRa.setPins(NSS_GPIO, RESET_GPIO, DIO0_GPIO);

  // Set frequency to the configured channel
  if (!LoRa.begin(channels[LORA_CHANNEL_INDEX])) {
    Serial.println("Starting LoRa failed!");
    screen_print("LoRa Failed!");
    return false;
  }

  // Enable CRC
  LoRa.enableCrc();
  
  // Set spreading factor
  LoRa.setSpreadingFactor(sf);

  // Set callback function for received packets
  LoRa.onReceive(onLoRaPacket);

  // Put LoRa module in receive mode
  LoRa.receive();
  
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Listening on %.1fMHz", (float)channels[LORA_CHANNEL_INDEX] / 1000000.0);
  Serial.println(buffer);
  screen_print("Sniffer ON");
  screen_print(buffer);
  screen_display();
  
  return true;
}