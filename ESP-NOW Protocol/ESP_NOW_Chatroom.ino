#include <WiFi.h>
#include <esp_now.h>

uint8_t peerMAC[] = {0x80, 0xB5, 0x4E, 0xC1, 0xB9, 0x60};  // Replace with peer MAC 

typedef struct struct_message {
  char msg[250];  // Max payload size
} struct_message;

struct_message myData;

// === Receive callback ===
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message incoming;
  memcpy(&incoming, incomingData, sizeof(incoming));
  Serial.print("[RECV] ");
  Serial.println(incoming.msg);
}

// === Send callback ===
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: print status
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SETUP] ESP-NOW Chat Ready");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed!");
    while (true);
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  Serial.println("[SETUP] Connecting to peer...");
  while (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WAIT] Peer not added, retrying...");
    delay(1000);
  }
  Serial.println("[SETUP] Peer connected!");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove trailing newline or spaces

    if (input.length() > 0) {
      strncpy(myData.msg, input.c_str(), sizeof(myData.msg));
      esp_now_send(peerMAC, (uint8_t *)&myData, sizeof(myData));
      Serial.print("[SEND] ");
      Serial.println(myData.msg);
    }
  }
}