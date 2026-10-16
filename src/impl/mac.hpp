#include <Arduino.h>
#include "esp_mac.h" // Built into the ESP32 core

void setup() {
  Serial.begin(115200);
  delay(1000);

  uint8_t mac[6];
  
  // Directly reads the factory base MAC address from chip eFuse
  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
    Serial.printf("ESP32 Hardware MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  } else {
    Serial.println("Failed to read MAC address from eFuse");
  }
}

void loop() {}
