// #include <WiFi.h>
// #include <esp_now.h>

// // Replace with the peer MAC address (the other board’s STA MAC)
// uint8_t peerMAC[] = {0x80, 0xB5, 0x4E, 0xC1, 0x15, 0x28};  // Example: Board B’s MAC
// // On Board B, replace with Board A’s MAC: {0x80, 0xB5, 0x4E, 0xC1, 0xB9, 0x60}

// typedef struct struct_message {
//   char msg[64];
// } struct_message;

// struct_message myData;

// // === Callbacks (Arduino core 2.x signatures) ===

// // Receive callback
// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//   struct_message incoming;
//   memcpy(&incoming, incomingData, sizeof(incoming));

//   Serial.print("[RECV] From MAC: ");
//   for (int i = 0; i < 6; i++) {
//     Serial.printf("%02X", mac[i]);
//     if (i < 5) Serial.print(":");
//   }

//   // Print message
//   Serial.print(" | Data: ");
//   Serial.println(incoming.msg);

//   // Print RSSI (signal strength)
//   Serial.print(" | RSSI: ");
//   Serial.println(WiFi.RSSI());
// }

// // Send callback
// void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//   Serial.print("[SEND] Status: ");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
// }

// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n[SETUP] Starting ESP-NOW demo...");

//   WiFi.mode(WIFI_STA);
//   delay(100);

//   if (esp_now_init() != ESP_OK) {
//     Serial.println("[ERROR] ESP-NOW init failed!");
//     return;
//   }
//   Serial.println("[SETUP] ESP-NOW initialized");

//   esp_now_register_recv_cb(OnDataRecv);
//   esp_now_register_send_cb(OnDataSent);

//   // Add peer (the other board)
//   esp_now_peer_info_t peerInfo = {};
//   memcpy(peerInfo.peer_addr, peerMAC, 6);
//   peerInfo.channel = 1;   // force same channel on both boards
//   peerInfo.encrypt = false;

//   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//     Serial.println("[ERROR] Failed to add peer");
//   } else {
//     Serial.println("[SETUP] Peer added successfully");
//   }
// }

// void loop() {
//   snprintf(myData.msg, sizeof(myData.msg), "Hello from %s", WiFi.macAddress().c_str());

//   Serial.print("[LOOP] Sending: ");
//   Serial.println(myData.msg);

//   esp_err_t result = esp_now_send(peerMAC, (uint8_t *)&myData, sizeof(myData));

//   if (result == ESP_OK) {
//     Serial.println("[LOOP] Send request queued");
//   } else {
//     Serial.print("[ERROR] Send failed, code: ");
//     Serial.println(result);
//   }

//   delay(2000);
// }

#include <WiFi.h>
#include <esp_now.h>

// === Change this MAC for each board ===
// Board A: peerMAC = Board B’s MAC
// Board B: peerMAC = Board A’s MAC
uint8_t peerMAC[] = {0x80, 0xB5, 0x4E, 0xC1, 0x15, 0x28};  // Example

typedef struct struct_message {
  char msg[64];
} struct_message;

struct_message myData;
unsigned long counter = 0;

// === Receive callback ===
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message incoming;
  memcpy(&incoming, incomingData, sizeof(incoming));

  // Print only what we receive
  Serial.println(incoming.msg);
}

// === Send callback (optional) ===
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Not printing here to keep output clean
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[SETUP] Starting ESP-NOW demo...");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed!");
    while (true); // stop here if init fails
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Try to connect to peer until success
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  Serial.println("[SETUP] Trying to connect to peer...");
  while (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WAIT] Peer not added, retrying...");
    delay(1000);
  }
  Serial.println("[SETUP] Peer connected successfully!");
}

void loop() {
  counter++;
  snprintf(myData.msg, sizeof(myData.msg), "Mine Found %lu", counter);

  esp_now_send(peerMAC, (uint8_t *)&myData, sizeof(myData));

  // Separator line to show loop activity
  Serial.println("-----------------------------");

  delay(2000); // send every 2 seconds
}
