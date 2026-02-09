#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

uint8_t remoteAddress[] = {0x24, 0x58, 0x7C, 0xD6, 0xF0, 0xF8};

const int pinX = 25;
const int pinY = 26;
const int pinLaser = 27;
const int CENTER_X = 85;
const int CENTER_Y = 91;

Servo servoX;
Servo servoY;
esp_now_peer_info_t peerInfo;

bool startSequence = false;

typedef struct cmd_struct {
  int command;
} cmd_struct;
cmd_struct incomingCmd;

typedef struct data_struct {
  int x;
  int y;
} data_struct;
data_struct reportData;

const int dataSize = 36;
int coordinates[dataSize][2] = {
  {131, 92}, {121, 92}, {109, 92}, {97, 91}, {85, 91}, {72, 91}, {62, 91}, {52, 91}, {41, 91},
  {86, 50}, {85, 59}, {85, 70}, {85, 80}, {85, 90}, {84, 101}, {84, 111}, {83, 122}, {82, 134},
  {134, 57}, {121, 63}, {109, 71}, {98, 80}, {83, 90}, {73, 100}, {61, 110}, {50, 118}, {40, 124},
  {130, 126}, {119, 118}, {108, 110}, {97, 100}, {83, 90}, {72, 80}, {62, 71}, {52, 63}, {41, 57}
};
int coordinates_send[dataSize][2] = {
  {-40, 0}, {-30, 0}, {-20, 0}, {-10, 0}, {0, 0}, {10, 0}, {20, 0}, {30, 0}, {40, 0},
  {0, -40}, {0, -30}, {0, -20}, {0, -10}, {0, 0}, {0, 10}, {0, 20}, {0, 30}, {0, 40},
  {-40, -40}, {-30, -30}, {-20, -20}, {-10, -10}, {0, 0}, {10, 10}, {20, 20}, {30, 30}, {40, 40},
  {-40, 40}, {-30, 30}, {-20, 20}, {-10, 10}, {0, 0}, {10, -10}, {20, -20}, {30, -30}, {40, -40}
};

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  memcpy(&incomingCmd, incomingData, sizeof(incomingCmd));
  if (incomingCmd.command == 1) {
    startSequence = true;
  }
}

void sendReport(int x, int y) {
  reportData.x = x;
  reportData.y = y;
  esp_now_send(remoteAddress, (uint8_t *) &reportData, sizeof(reportData));
}

void setup() {
  Serial.begin(115200);

  pinMode(pinLaser, OUTPUT);
  digitalWrite(pinLaser, LOW);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);
  servoX.attach(pinX, 500, 2400);
  servoY.attach(pinY, 500, 2400);

  servoX.write(CENTER_X);
  servoY.write(CENTER_Y);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, remoteAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void executeRoutine() {
  Serial.println("Start Routine...");
  
  for (int i = 0; i < dataSize; i++) {
    int targetX = coordinates[i][0];
    int targetY = coordinates[i][1];
    int sendX = coordinates_send[i][0];
    int sendY = coordinates_send[i][1];
    digitalWrite(pinLaser, LOW);
    servoX.write(targetX);
    servoY.write(targetY);
    delay(600);

    digitalWrite(pinLaser, HIGH);

    delay(3000);

    sendReport(sendX, sendY);
  }
  digitalWrite(pinLaser, LOW);
  servoX.write(CENTER_X);
  servoY.write(CENTER_Y);
  sendReport(CENTER_X, CENTER_Y);
  startSequence = false;
}

void loop() {
  if (startSequence) {
    executeRoutine();
  }
}