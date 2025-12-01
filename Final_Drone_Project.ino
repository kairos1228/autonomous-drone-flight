//-----------------------------------------------------
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
unsigned int firstRoll;
unsigned int firstPitch;
//===== 드론 2호 제어에 필요한 변수 선언 부분 끝 =====

//-----------------------------------------------------

//=========== 드론 2호 높이 제어 부분 시작 ===========
void checkThrottle()
{
  
  //throttle 값 감소 : 드론 2호 하강 제어 부분
  if(!digitalRead(6))
  {
    if(throttle > 9)
      throttle -= 10;
  }
  
  //throttle 값 증가 : 드론 2호 상승 제어 부분
  else if(!digitalRead(5))
  {
    if(throttle < 141)
      throttle += 20;
  }
}
//============ 드론 2호 높이 제어 부분 끝 ============

//-----------------------------------------------------

//=== 드론 2호 좌회전/우회전(제자리) 제어 부분 시작 ===
void checkYaw()
{
  if(throttle == 0)
    yaw = 0;
    
  //yaw 값 감소 : 드론 2호 좌회전(제자리) 제어 부분
  if(!digitalRead(7))
  {
    if(yaw > -170)
      yaw -= 10;
  }
  
  //yaw 값 증가 : 드론 2호 우회전(제자리) 제어 부분
  else if(!digitalRead(8))
  {
    if(yaw < 170)
      yaw += 10;
  }
}
//==== 드론 2호 좌회전/우회전(제자리) 제어 부분 끝 ====

//-----------------------------------------------------

//========= 드론 2호 비상 착륙 제어 부분 시작 =========
void checkEmergency()
{
  //비상버튼 눌리는지 체크하는 부분
  if(!digitalRead(9))
  {
    roll = 0;
    pitch = 0;
    yaw = 0;
    throttle = 0;
    //
    option = 0x000e;
  }
  else
  {
    option = 0x000f;
  }
}
//========== 드론 2호 비상 착륙 제어 부분 끝 ==========

//-----------------------------------------------------

//====== 드론 2호 좌측/우측 이동 제어 부분 시작 ======
void checkRoll()
{
  //roll 값 증가 : 오른쪽 이동, roll 값 감소 : 왼쪽 이동
  unsigned int secondRoll = analogRead(4);

  if(secondRoll < firstRoll - 450)
    roll = -200;
  else if(secondRoll < firstRoll - 350)
    roll = -160;
  else if(secondRoll < firstRoll - 250)
    roll = -120;
  else if(secondRoll < firstRoll - 150)
    roll = -80;
  else if(secondRoll < firstRoll - 50)
    roll = -40;
  else if(secondRoll < firstRoll + 50)
    roll = 0;
  else if(secondRoll < firstRoll + 150)
    roll = 40;
  else if(secondRoll < firstRoll + 250)
    roll = 80;
  else if(secondRoll < firstRoll + 350)
    roll = 120;
  else if(secondRoll < firstRoll + 450)
    roll = 160;
  else
    roll = 200;
}
//======= 드론 2호 좌측/우측 이동 제어 부분 끝 =======

//-----------------------------------------------------

//====== 드론 2호 전진/후진 이동 제어 부분 시작 ======
void checkPitch()
{
  //pitch 값 증가 : 앞으로 이동, pitch 값 감소 : 뒤로 이동
  unsigned int secondPitch = analogRead(5);

  if(secondPitch < firstPitch - 450)
    pitch = -200;
  else if(secondPitch < firstPitch - 350)
    pitch = -160;
  else if(secondPitch < firstPitch - 250)
    pitch = -120;
  else if(secondPitch < firstPitch - 150)
    pitch = -80;
  else if(secondPitch < firstPitch - 50)
    pitch = -40;
  else if(secondPitch < firstPitch + 50)
    pitch = 0;
  else if(secondPitch < firstPitch + 150)
    pitch = 40;
  else if(secondPitch < firstPitch + 250)
    pitch = 80;
  else if(secondPitch < firstPitch + 350)
    pitch = 120;
  else if(secondPitch < firstPitch + 450)
    pitch = 160;
  else
    pitch = 200;
}
//======= 드론 2호 전진/후진 이동 제어 부분 끝 =======

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

//==== 드론 2호 제어 명령 데이터 첫 진행 부분 시작 ====
unsigned char startDroneControl()
{
   if(!digitalRead(9))
   {
     firstRoll = analogRead(4);
     firstPitch = analogRead(5);
     drone_action = 1;
   }
   return drone_action;
}
//===== 드론 2호 제어 명령 데이터 첫 진행 부분 끝 =====

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
  if(startDroneControl())
  {
    checkThrottle();
    checkRoll();
    checkPitch();
    checkYaw();
    checkEmergency();
    //
    checkCRC();
    //
    sendDroneCommand();
  }
}
//========= 아두이노 우노의 무한 반복 부분 끝 =========
