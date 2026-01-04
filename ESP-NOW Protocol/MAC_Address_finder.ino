#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SETUP] Starting MAC address demo...");

  // Step 1: Set Wi-Fi mode to Station
  WiFi.mode(WIFI_STA);
  delay(100);  // give radio time to initialize

  // Step 2: Get and print MAC address
  String mac = WiFi.macAddress();
  Serial.print("[INFO] ESP32-S3 MAC Address: ");
  Serial.println(mac);
}

void loop() {
  // Nothing else needed — MAC is printed once at startup
}
