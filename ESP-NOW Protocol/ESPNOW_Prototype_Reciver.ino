/*
  ESP-NOW Receiver
  Receives detection data (Label + Confidence) from Edge Impulse Camera
*/

#include <esp_now.h>
#include <WiFi.h>

// 1. DATA STRUCTURE
// Must match the sender structure exactly
typedef struct struct_message {
  char label[32];     // Name of the detected object
  float confidence;   // Confidence score
} struct_message;

// Create a struct_message called myData
struct_message myData;

// 2. CALLBACK FUNCTION
// This function is executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.print("Bytes received: ");
  Serial.println(len);
  
  Serial.print("Detection: ");
  Serial.println(myData.label);
  
  Serial.print("Confidence: ");
  Serial.println(myData.confidence);
  
  Serial.print("From MAC: ");
  for(int i=0; i<6; i++){
     Serial.printf("%02X", mac[i]);
     if(i<5) Serial.print(":");
  }
  Serial.println();
  Serial.println("-----------------------");
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register for recv CB to get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP-NOW Receiver initialized. Waiting for data...");
}

void loop() {
  // Relax, everything happens in the callback
  delay(1000);
}