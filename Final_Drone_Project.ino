//-----------------------------------------------------
// 자율 경로비행 드론 제어 프로그램
// 2m x 3m 직사각형 경로 (1→2→3→4→1)
//-----------------------------------------------------

#include <SoftwareSerial.h>

SoftwareSerial bleSerial(A0, A1); // RX, TX 

//-----------------------------------------------------

//==== 드론 2호 제어에 필요한 변수 선언 부분 시작 ====
unsigned char startBit_1 = 0x26;
unsigned char startBit_2 = 0xa8;
unsigned char startBit_3 = 0x14;
unsigned char startBit_4 = 0xb1;
unsigned char len = 0x14;
unsigned char checkSum = 0;
//
int roll = 0;
int pitch = 0;
int yaw = 0;
int throttle = 0;
int option = 0x000f;
//
int p_vel = 0x0064;
int y_vel = 0x0064;

unsigned char drone_action = 0;
unsigned char payload[14];
//===== 드론 2호 제어에 필요한 변수 선언 부분 끝 =====

//-----------------------------------------------------

//========== 자율비행 제어 변수 선언 부분 시작 ==========
// 비행 단계 정의
enum FlightPhase {
  IDLE,           // 대기
  TAKEOFF,        // 이륙
  HOVER_START,    // 시작점 호버링
  MOVE_TO_P2,     // 지점 2로 이동
  HOVER_P2,       // 지점 2 호버링
  MOVE_TO_P3,     // 지점 3으로 이동
  HOVER_P3,       // 지점 3 호버링
  MOVE_TO_P4,     // 지점 4로 이동
  HOVER_P4,       // 지점 4 호버링
  MOVE_TO_P1,     // 시작점으로 복귀
  HOVER_FINAL,    // 최종 호버링
  LANDING,        // 착륙
  COMPLETED       // 완료
};

FlightPhase currentPhase = IDLE;
unsigned long phaseStartTime = 0;

// 비행 파라미터
const int CRUISE_ALTITUDE = 140;     // 순항 고도 (더 높은 고도로 증가)
const int FLIGHT_SPEED = 80;         // 비행 속도 (안정성을 위해 약간 감소)
const int HOVER_TIME = 500;          // 각 지점 호버링 시간 (ms)

// 이동 시간 계산 (ms)
// 속도와 거리를 고려한 실험적 값
const unsigned long TIME_TO_P2 = 3500;   // 3m 이동 (우측)
const unsigned long TIME_TO_P3 = 2500;   // 2m 이동 (전진)
const unsigned long TIME_TO_P4 = 3500;   // 3m 이동 (좌측)
const unsigned long TIME_TO_P1 = 2500;   // 2m 이동 (후진)

// 시작 버튼 플래그
bool flightStarted = false;
//========== 자율비행 제어 변수 선언 부분 끝 ==========

//-----------------------------------------------------

//============ 자율비행 제어 함수 부분 시작 ============

// 드론 정지 (호버링)
void hoverDrone()
{
  roll = 0;
  pitch = 0;
  yaw = 0;
  throttle = CRUISE_ALTITUDE;
}

// 우측으로 이동 (지점 1 → 2)
void moveRight()
{
  roll = FLIGHT_SPEED;
  pitch = 0;
  yaw = 0;
  throttle = CRUISE_ALTITUDE;
}

// 전진 (지점 2 → 3)
void moveForward()
{
  roll = 0;
  pitch = FLIGHT_SPEED;
  yaw = 0;
  throttle = CRUISE_ALTITUDE;
}

// 좌측으로 이동 (지점 3 → 4)
void moveLeft()
{
  roll = -FLIGHT_SPEED;
  pitch = 0;
  yaw = 0;
  throttle = CRUISE_ALTITUDE;
}

// 후진 (지점 4 → 1)
void moveBackward()
{
  roll = 0;
  pitch = -FLIGHT_SPEED;
  yaw = 0;
  throttle = CRUISE_ALTITUDE;
}

// 이륙 시퀀스
void takeoff()
{
  roll = 0;
  pitch = 0;
  yaw = 0;

  // 부드러운 이륙을 위한 단계적 상승
  unsigned long elapsedTime = millis() - phaseStartTime;

  if(elapsedTime < 1000) {
    throttle = 60;  // 초기 상승 (더 강하게)
  }
  else if(elapsedTime < 2000) {
    throttle = 100;  // 중간 고도 (더 높이)
  }
  else {
    throttle = CRUISE_ALTITUDE;  // 순항 고도
  }
}

// 착륙 시퀀스
void land()
{
  roll = 0;
  pitch = 0;
  yaw = 0;

  // 부드러운 착륙을 위한 단계적 하강
  unsigned long elapsedTime = millis() - phaseStartTime;

  if(elapsedTime < 1000) {
    throttle = 100;  // 천천히 하강 시작
  }
  else if(elapsedTime < 2000) {
    throttle = 60;  // 중간 고도
  }
  else if(elapsedTime < 3000) {
    throttle = 30;  // 저고도
  }
  else {
    throttle = 0;   // 완전 착륙
  }
}

// 비상 정지
void emergencyStop()
{
  roll = 0;
  pitch = 0;
  yaw = 0;
  throttle = 0;
  option = 0x000e;
  currentPhase = COMPLETED;
}
//============ 자율비행 제어 함수 부분 끝 ============

//-----------------------------------------------------

//========= 드론 2호 제어 명령 구조 부분 시작 =========
void sendDroneCommand()
{
  bleSerial.print("at+writeh000d");
  bleSerial.print(String(startBit_1, HEX));
  bleSerial.print(String(startBit_2, HEX));
  bleSerial.print(String(startBit_3, HEX));
  bleSerial.print(String(startBit_4, HEX));
  bleSerial.print(String(len, HEX));
  //checkSum
  if(checkSum < 0x10)
    bleSerial.print("0" + String(checkSum, HEX));
  else
    bleSerial.print(String(checkSum, HEX));
  //roll
  if(payload[0] < 0x10)
    bleSerial.print("0" + String(payload[0], HEX));
  else
    bleSerial.print(String(payload[0], HEX));
  if(payload[1] < 0x10)
    bleSerial.print("0" + String(payload[1], HEX));
  else
    bleSerial.print(String(payload[1], HEX));
  //pitch
  if(payload[2] < 0x10)
    bleSerial.print("0" + String(payload[2], HEX));
  else
    bleSerial.print(String(payload[2], HEX));
  if(payload[3] < 0x10)
    bleSerial.print("0" + String(payload[3], HEX));
  else
    bleSerial.print(String(payload[3], HEX));
  //yaw
  if(payload[4] < 0x10)
    bleSerial.print("0" + String(payload[4], HEX));
  else
    bleSerial.print(String(payload[4], HEX));
  if(payload[5] < 0x10)
    bleSerial.print("0" + String(payload[5], HEX));
  else
    bleSerial.print(String(payload[5], HEX));
  //throttle
  if(payload[6] < 0x10)
    bleSerial.print("0" + String(payload[6], HEX));
  else
    bleSerial.print(String(payload[6], HEX));
  if(payload[7] < 0x10)
    bleSerial.print("0" + String(payload[7], HEX));
  else
    bleSerial.print(String(payload[7], HEX));
  //option
  if(payload[8] < 0x10)
    bleSerial.print("0" + String(payload[8], HEX));
  else
    bleSerial.print(String(payload[8], HEX));
  if(payload[9] < 0x10)
    bleSerial.print("0" + String(payload[9], HEX));
  else
    bleSerial.print(String(payload[9], HEX));
  //p_vel
  if(payload[10] < 0x10)
    bleSerial.print("0" + String(payload[10], HEX));
  else
    bleSerial.print(String(payload[10], HEX));
  if(payload[11] < 0x10)
    bleSerial.print("0" + String(payload[11], HEX));
  else
    bleSerial.print(String(payload[11], HEX));
  //y_vel
  if(payload[12] < 0x10)
    bleSerial.print("0" + String(payload[12], HEX));
  else
    bleSerial.print(String(payload[12], HEX));
  if(payload[13] < 0x10)
    bleSerial.print("0" + String(payload[13], HEX));
  else
    bleSerial.print(String(payload[13], HEX));
  //
  bleSerial.print("\r");
  delay(50);
}
//========== 드론 2호 제어 명령 구조 부분 끝 ==========

//-----------------------------------------------------

//=== 데이터 전송시 오류 검출을 하기 위한 부분 시작 ===
void checkCRC()
{
  memset(payload, 0x00, 14);
  //
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
  //
  payload[10] = (p_vel) & 0x00ff;
  payload[11] = (p_vel >> 8) & 0x00ff;
  payload[12] = (y_vel) & 0x00ff;
  payload[13] = (y_vel >> 8) & 0x00ff;
  //
  checkSum = 0;
  for(int i = 0; i < 14; i++)
    checkSum += payload[i];
  checkSum = checkSum & 0x00ff;
}
//==== 데이터 전송시 오류 검출을 하기 위한 부분 끝 ====

//-----------------------------------------------------

//======== 자율비행 상태 머신 제어 부분 시작 ========
void updateFlightPhase()
{
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - phaseStartTime;

  // 비상정지 버튼 체크 (버튼 9)
  if(!digitalRead(9) && currentPhase != IDLE && currentPhase != COMPLETED) {
    Serial.println("Emergency Stop!");
    emergencyStop();
    return;
  }

  switch(currentPhase) {
    case IDLE:
      // 시작 버튼 대기 (버튼 5)
      if(!digitalRead(5)) {
        Serial.println("Flight Started!");
        currentPhase = TAKEOFF;
        phaseStartTime = currentTime;
        drone_action = 1;
      }
      break;

    case TAKEOFF:
      takeoff();
      if(elapsedTime > 2500) {  // 2.5초 이륙
        Serial.println("Takeoff Complete - Hovering at Start");
        currentPhase = HOVER_START;
        phaseStartTime = currentTime;
      }
      break;

    case HOVER_START:
      hoverDrone();
      if(elapsedTime > HOVER_TIME) {
        Serial.println("Moving to Point 2 (Right)");
        currentPhase = MOVE_TO_P2;
        phaseStartTime = currentTime;
      }
      break;

    case MOVE_TO_P2:
      moveRight();
      if(elapsedTime > TIME_TO_P2) {
        Serial.println("Reached Point 2 - Hovering");
        currentPhase = HOVER_P2;
        phaseStartTime = currentTime;
      }
      break;

    case HOVER_P2:
      hoverDrone();
      if(elapsedTime > HOVER_TIME) {
        Serial.println("Moving to Point 3 (Forward)");
        currentPhase = MOVE_TO_P3;
        phaseStartTime = currentTime;
      }
      break;

    case MOVE_TO_P3:
      moveForward();
      if(elapsedTime > TIME_TO_P3) {
        Serial.println("Reached Point 3 - Hovering");
        currentPhase = HOVER_P3;
        phaseStartTime = currentTime;
      }
      break;

    case HOVER_P3:
      hoverDrone();
      if(elapsedTime > HOVER_TIME) {
        Serial.println("Moving to Point 4 (Left)");
        currentPhase = MOVE_TO_P4;
        phaseStartTime = currentTime;
      }
      break;

    case MOVE_TO_P4:
      moveLeft();
      if(elapsedTime > TIME_TO_P4) {
        Serial.println("Reached Point 4 - Hovering");
        currentPhase = HOVER_P4;
        phaseStartTime = currentTime;
      }
      break;

    case HOVER_P4:
      hoverDrone();
      if(elapsedTime > HOVER_TIME) {
        Serial.println("Returning to Point 1 (Backward)");
        currentPhase = MOVE_TO_P1;
        phaseStartTime = currentTime;
      }
      break;

    case MOVE_TO_P1:
      moveBackward();
      if(elapsedTime > TIME_TO_P1) {
        Serial.println("Returned to Start - Final Hovering");
        currentPhase = HOVER_FINAL;
        phaseStartTime = currentTime;
      }
      break;

    case HOVER_FINAL:
      hoverDrone();
      if(elapsedTime > HOVER_TIME) {
        Serial.println("Starting Landing");
        currentPhase = LANDING;
        phaseStartTime = currentTime;
      }
      break;

    case LANDING:
      land();
      if(elapsedTime > 3500) {  // 3.5초 착륙 (더 부드럽게)
        Serial.println("Flight Completed!");
        currentPhase = COMPLETED;
        option = 0x000e;  // 모터 정지
      }
      break;

    case COMPLETED:
      roll = 0;
      pitch = 0;
      yaw = 0;
      throttle = 0;
      option = 0x000e;
      break;
  }
}
//========= 자율비행 상태 머신 제어 부분 끝 =========

//-----------------------------------------------------

//======== 아두이노 우노의 초기 설정 부분 시작 ========
void setup()
{
  Serial.begin(9600);
  Serial.println("Test Started!");
  bleSerial.begin(9600);

  for(int i = 5; i < 11; i++)
  {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  delay(500);
}
//========= 아두이노 우노의 초기 설정 부분 끝 =========

//-----------------------------------------------------

//======== 아두이노 우노의 무한 반복 부분 시작 ========
void loop()
{
  // 자율비행 상태 머신 업데이트
  updateFlightPhase();

  // 드론 제어 명령 전송 (IDLE 상태가 아닐 때만)
  if(currentPhase != IDLE) {
    checkCRC();
    sendDroneCommand();
  }
}
//========= 아두이노 우노의 무한 반복 부분 끝 =========
