# 자율 경로비행 드론 프로젝트

마이크로컨트롤러 수업 기말 프로젝트 - 3.12m x 2.61m 직사각형 경로 자율비행

## 프로젝트 개요

Arduino 기반 드론이 실측 보정된 직사각형 경로를 정밀하게 자율비행하여 시작점으로 복귀하는 시스템입니다.

## 비행 경로 (실측 보정판)

```
    4 ←──────── 3
    │   3.12m   ↑
2.61m │         │ 2.61m
    ↓           │
    1 ────────→ 2
       3.12m
```

- 시작점 (Point 1): 이륙 및 착륙 위치
- Point 2: 전진 3.12m 이동
- Point 3: 우측으로 2.61m 이동
- Point 4: 후진 3.12m 이동
- Point 1: 좌측으로 2.61m로 시작점 복귀

## 주요 사양

### 비행 파라미터 (실측 보정)
- **순항 고도**: 120cm (ALTHOLD 모드)
- **이동 파워**: 100 (고정값)
- **전진/후진 시간**: 2400ms (3.12m 기준)
- **좌우 이동 시간**: 2450ms (2.61m 기준)
- **안정화 시간**: 각 동작 후 2초

### 비행 시퀀스
1. **이륙 및 고도 고정 (5초)**: ALTHOLD 모드로 120cm 고정
2. **경로 비행**: 실측 기반 타이밍으로 정밀 이동
3. **소프트 랜딩**: 30ms마다 1cm씩 부드러운 하강

## 하드웨어 구성

- Arduino Uno
- 드론 2호 (BLE 통신)
- HC-06 블루투스 모듈 (A0: RX, A1: TX)
- 제어 버튼
  - Pin 9: Emergency 버튼 (시작 버튼으로 사용)

## 사용 방법

### 1. 준비
1. Arduino에 코드 업로드
2. 드론과 블루투스 페어링 확인
3. 드론을 시작 지점 (Point 1)에 위치
4. 시리얼 모니터 연결 (9600 baud)

### 2. 비행 시작
1. Pin 9 (Emergency) 버튼을 눌러 자율비행 시작
2. 드론이 자동으로 이륙 → 고도 고정 (5초)
3. 실측 경로 비행: 전진 → 우측 → 후진 → 좌측
4. 시작점 복귀 후 소프트 랜딩

### 3. 안전 기능
- ALTHOLD 모드로 고도 자동 유지
- 각 동작 후 2초 안정화 시간
- 부드러운 소프트 랜딩 알고리즘

## 코드 구조

### 상태 머신 (State Machine)

```
State 0: 대기 (Emergency 버튼 입력 대기)
   ↓
State 1: 이륙 및 고도 고정 (5초, ALTHOLD 120cm)
   ↓
State 2: 전진 이동 (2443ms, Power 100)
   ↓
State 3: 안정화 (2초)
   ↓
State 4: 우측 이동 (2337ms, Power 100)
   ↓
State 5: 안정화 (2초)
   ↓
State 6: 후진 이동 (2443ms, Power 100)
   ↓
State 7: 안정화 (2초)
   ↓
State 8: 좌측 이동 (2337ms, Power 100)
   ↓
State 9: 소프트 랜딩 (30ms당 1cm 하강)
   ↓
State 10: 완전 종료
```

### 주요 함수
- `sendDroneCommand()`: BLE 프로토콜로 드론에 명령 전송
- `checkCRC()`: 데이터 무결성 검증을 위한 체크섬 계산
- `loop()`: 상태 머신 기반 자율비행 제어

## 성능 최적화

### 정밀도 향상
- **실측 기반 보정**: 실제 비행 데이터로 이동 시간 정밀 보정
- **ALTHOLD 모드**: 자동 고도 유지로 일정한 고도 유지
- **고정 파워값**: Power 100 고정으로 일관된 이동 속도

### 안정성 향상
- 각 동작 후 2초 안정화 시간으로 드리프트 최소화
- 소프트 랜딩 알고리즘으로 충격 없는 착륙
- 상태 머신 기반 제어로 예측 가능한 동작

## 실측 보정 데이터

### 측정 환경
- 고도: 120cm (ALTHOLD)
- 파워: 100 (고정)

### 보정된 시간 값
- **전진/후진 (3.12m)**: 2443ms
  - 기존 예측: 2357ms
  - 실측 평균으로 보정
- **좌우 이동 (2.61m)**: 2337ms
  - 기존 예측: 2301ms
  - 실측 평균으로 보정

## 튜닝 가이드

추가 보정이 필요한 경우 다음 값들을 조정할 수 있습니다:

```cpp
// 이동 파워 조정 (현재: 100 고정)
#define MOVE_POWER 100      // 80~120 범위 권장

// 목표 고도 조정
#define HOVER_HEIGHT 120    // 100~150cm 범위 권장

// 이동 시간 미세 조정 (실측 데이터 기반)
#define TIME_FORWARD  2443  // 3.12m 전진/후진
#define TIME_RIGHT    2337  // 2.61m 좌우 이동

// 안정화 시간 조정
#define TIME_HOVER    2000  // 동작 사이 안정화 (ms)
```

## 시리얼 모니터 출력

비행 중 다음과 같은 상태 메시지가 출력됩니다:

```
Calibrated System Ready. Press Emergency Button (D9) to START.
State 1: Takeoff & Altitude Fix (1.2m)
State 2: Moving Forward (3.12m)
State 3: Hovering (Stabilize)
State 4: Moving Right (2.61m)
State 5: Hovering (Stabilize)
State 6: Moving Backward (3.12m)
State 7: Hovering (Stabilize)
State 8: Moving Left (2.61m)
State 9: Soft Landing...
State 10: Shutdown
```

## 라이선스

교육용 프로젝트
