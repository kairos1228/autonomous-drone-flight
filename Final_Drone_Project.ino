/*
 * 드론 2호 자율주행 최종 코드 (3.12m x 2.61m 정밀 보정판)
 * - 고도: 120cm (ALTHOLD) / Power 100 사용
 * - 실측 기반 시간 보정 완료:
 * [전진] 3.12m -> 2357ms
 * [우측] 2.61m -> 2301ms
 */

#include <SoftwareSerial.h>

SoftwareSerial bleSerial(A0, A1); // RX, TX

// --- [사용자 설정 값 (보정됨)] ---
#define MOVE_POWER 100      // 이동 파워 고정
#define HOVER_HEIGHT 120    // 목표 유지 고도 (120cm)

// 실측 데이터 기반 계산된 시간  
#define TIME_FORWARD  2400  // 3.12m 이동 시간 (ms)
#define TIME_RIGHT    2450  // 2.61m 이동 시간 (ms)
#define TIME_HOVER    2000  // 동작 사이 안정화 시간 (2초)

// --- 변수 선언 ---
unsigned char startBit_1 = 0x26;
unsigned char startBit_2 = 0xa8;
unsigned char startBit_3 = 0x14;
unsigned char startBit_4 = 0xb1;
unsigned char len = 0x14;
unsigned char checkSum = 0;

int roll = 0;
int pitch = 0;
int yaw = 0;
int throttle = 0;
int option = 0x000e; // 초기값: 시동 꺼짐 (Emergency)

int p_vel = 0x0064;
int y_vel = 0x0064;

unsigned char payload[14];

// --- 상태 머신 ---
unsigned long timerStart = 0;
int flightState = 0; 

void setup() {
  Serial.begin(9600);
  bleSerial.begin(9600);
  
  // 핀 설정
  for(int i = 5; i < 11; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // 풀업
  }
  delay(500);
  Serial.println("Calibrated System Ready. Press Emergency Button (D9) to START.");
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long timeElapsed = currentTime - timerStart;

  switch (flightState) {
    
    // [단계 0] 대기 (시작 신호 대기)
    case 0:
      option = 0x000e; // 시동 OFF
      roll = 0; pitch = 0; yaw = 0; throttle = 0;
      
      if (!digitalRead(9)) { // 버튼 눌림 감지
        flightState = 1;
        timerStart = currentTime;
        Serial.println("State 1: Takeoff & Altitude Fix (1.2m)");
      }
      break;

    // [단계 1] 이륙 및 고도 고정 (5초 대기)
    case 1:
      option = 0x000f; // 비행 모드 ON (ALTHOLD 활성화)
      throttle = HOVER_HEIGHT; // 120cm 고도 명령
      roll = 0; pitch = 0; yaw = 0;

      if (timeElapsed > 5000) { 
        flightState = 2;
        timerStart = currentTime;
        Serial.println("State 2: Moving Forward (3.12m)");
      }
      break;

    // [단계 2] 전진 (2357ms)
    case 2:
      throttle = HOVER_HEIGHT; 
      pitch = MOVE_POWER;
      roll = 0;

      if (timeElapsed > TIME_FORWARD) {
        flightState = 3;
        timerStart = currentTime;
        Serial.println("State 3: Hovering (Stabilize)");
      }
      break;

    // [단계 3] 정지 (2초)
    case 3:
      throttle = HOVER_HEIGHT;
      pitch = 0; roll = 0;

      if (timeElapsed > TIME_HOVER) {
        flightState = 4;
        timerStart = currentTime;
        Serial.println("State 4: Moving Right (2.61m)");
      }
      break;

    // [단계 4] 우측 이동 (2301ms)
    case 4:
      throttle = HOVER_HEIGHT;
      pitch = 0;
      roll = MOVE_POWER;

      if (timeElapsed > TIME_RIGHT) {
        flightState = 5;
        timerStart = currentTime;
        Serial.println("State 5: Hovering (Stabilize)");
      }
      break;

    // [단계 5] 정지 (2초)
    case 5:
      throttle = HOVER_HEIGHT;
      pitch = 0; roll = 0;

      if (timeElapsed > TIME_HOVER) {
        flightState = 6;
        timerStart = currentTime;
        Serial.println("State 6: Moving Backward (3.12m)");
      }
      break;

    // [단계 6] 후진 (2357ms)
    case 6:
      throttle = HOVER_HEIGHT;
      pitch = -MOVE_POWER;
      roll = 0;

      if (timeElapsed > TIME_FORWARD) {
        flightState = 7;
        timerStart = currentTime;
        Serial.println("State 7: Hovering (Stabilize)");
      }
      break;

    // [단계 7] 정지 (2초)
    case 7:
      throttle = HOVER_HEIGHT;
      pitch = 0; roll = 0;

      if (timeElapsed > TIME_HOVER) {
        flightState = 8;
        timerStart = currentTime;
        Serial.println("State 8: Moving Left (2.61m)");
      }
      break;

    // [단계 8] 좌측 이동 (2301ms)
    case 8:
      throttle = HOVER_HEIGHT;
      pitch = 0;
      roll = -MOVE_POWER;

      if (timeElapsed > TIME_RIGHT) {
        flightState = 9;
        timerStart = currentTime;
        Serial.println("State 9: Soft Landing...");
      }
      break;

    // [단계 9] 소프트 랜딩 (서서히 하강)
    case 9:
      option = 0x000f; 
      pitch = 0; roll = 0; yaw = 0;
      
      // 30ms마다 목표 고도를 1cm씩 낮춤
      {
        int landingHeight = HOVER_HEIGHT - (int)(timeElapsed / 30);
        
        if (landingHeight > 0) {
          throttle = landingHeight; 
        } else {
          throttle = 0;
          flightState = 10;
          timerStart = currentTime;
          Serial.println("State 10: Shutdown");
        }
      }
      break;

    // [단계 10] 완전 종료
    case 10:
      option = 0x000e; // 시동 OFF
      throttle = 0; pitch = 0; roll = 0;
      break;
  }

  // --- 명령 전송 ---
  checkCRC();
  sendDroneCommand();
}

// -----------------------------------------------------------
// 통신 프로토콜 (수정 없음)
// -----------------------------------------------------------
void sendDroneCommand() {
  bleSerial.print("at+writeh000d");
  bleSerial.print(String(startBit_1, HEX));
  bleSerial.print(String(startBit_2, HEX));
  bleSerial.print(String(startBit_3, HEX));
  bleSerial.print(String(startBit_4, HEX));
  bleSerial.print(String(len, HEX));

  if(checkSum < 0x10) bleSerial.print("0" + String(checkSum, HEX));
  else bleSerial.print(String(checkSum, HEX));

  for(int i=0; i<14; i++) {
    if(payload[i] < 0x10) bleSerial.print("0" + String(payload[i], HEX));
    else bleSerial.print(String(payload[i], HEX));
  }
  bleSerial.print("\r");
  delay(50);
}

void checkCRC() {
  memset(payload, 0x00, 14);
  payload[0] = (roll) & 0x00ff;
  payload[1] = (roll >> 8) & 0x00ff;
  payload[2] = (pitch) & 0x00ff;
  payload[3] = (pitch >> 8) & 0x00ff;
  payload[4] = (yaw) & 0x00ff;
  payload[5] = (yaw >> 8) & 0x00ff;
  payload[6] = (throttle) & 0x00ff;
  payload[7] = (throttle >> 8) & 0x00ff;
  payload[8] = (option) & 0x00ff;
  payload[9] = (option >> 8) & 0x00ff;
  payload[10] = (p_vel) & 0x00ff;
  payload[11] = (p_vel >> 8) & 0x00ff;
  payload[12] = (y_vel) & 0x00ff;
  payload[13] = (y_vel >> 8) & 0x00ff;

  checkSum = 0;
  for(int i = 0; i < 14; i++) checkSum += payload[i];
  checkSum = checkSum & 0x00ff;
}