#include <WiFi.h>
#include <esp_now.h>

// Message structure
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message myData;

// === Callbacks (old signatures for Arduino core 2.x) ===

// Receive callback
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message incoming;
  memcpy(&incoming, incomingData, sizeof(incoming));
  Serial.print("[RECV] From MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Data: ");
  Serial.println(incoming.msg);
}

// Send callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("[SEND] Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SETUP] Starting ESP-NOW broadcast demo...");

  // Step 1: Set Wi-Fi mode
  WiFi.mode(WIFI_STA);
  delay(100);  // give radio time to initialize
  Serial.println("[SETUP] WiFi set to STA mode");

  // Step 2: Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed!");
    return;
  }
  Serial.println("[SETUP] ESP-NOW initialized");

  // Step 3: Register callbacks
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  Serial.println("[SETUP] Callbacks registered");

  // Step 4: Add broadcast peer
  esp_now_peer_info_t peerInfo = {};
  memset(peerInfo.peer_addr, 0xFF, 6);  // broadcast address
  peerInfo.channel = 1;                 // force channel 1 for both boards
  peerInfo.encrypt = false;

  esp_err_t addStatus = esp_now_add_peer(&peerInfo);
  if (addStatus == ESP_OK) {
    Serial.println("[SETUP] Broadcast peer added successfully");
  } else {
    Serial.print("[ERROR] Failed to add peer, code: ");
    Serial.println(addStatus);
  }
}

void loop() {
  snprintf(myData.msg, sizeof(myData.msg), "Hello from %s", WiFi.macAddress().c_str());

  Serial.print("[LOOP] Sending: ");
  Serial.println(myData.msg);

  esp_err_t result = esp_now_send(NULL, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("[LOOP] Send request queued");
  } else {
    Serial.print("[ERROR] Send failed, code: ");
    Serial.println(result);

    // Decode common errors
    if (result == ESP_ERR_ESPNOW_NOT_INIT) Serial.println("Reason: ESP-NOW not initialized");
    else if (result == ESP_ERR_ESPNOW_ARG) Serial.println("Reason: Invalid argument");
    else if (result == ESP_ERR_ESPNOW_INTERNAL) Serial.println("Reason: Internal error");
    else if (result == ESP_ERR_ESPNOW_NO_MEM) Serial.println("Reason: Out of memory");
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND) Serial.println("Reason: Peer not found");
    else if (result == ESP_ERR_ESPNOW_IF) Serial.println("Reason: Invalid interface");
  }

  delay(2000);
}