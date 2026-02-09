#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

uint8_t robotAddress[] = {0x08, 0xF9, 0xE0, 0xBD, 0xE9, 0xA0}; 

typedef struct cmd_struct {
  int command;
} cmd_struct;
cmd_struct myCommand;

typedef struct data_struct {
  int x;
  int y;
} data_struct;
data_struct incomingData;

esp_now_peer_info_t peerInfo;

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  Serial.printf("X=%d, Y=%d\n", incomingData.x, incomingData.y);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, robotAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar == '1') {
      myCommand.command = 1;
      esp_now_send(robotAddress, (uint8_t *) &myCommand, sizeof(myCommand));
      Serial.println("Start.");
    }
    else if (inChar == '2') {
      myCommand.command = 2;
      esp_now_send(robotAddress, (uint8_t *) &myCommand, sizeof(myCommand));
      Serial.println("Emergency stop!");
    }
  }
}