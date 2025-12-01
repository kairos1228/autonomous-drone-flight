# 자율 경로비행 드론 프로젝트

마이크로컨트롤러 수업 기말 프로젝트 - 2m x 3m 직사각형 경로 자율비행

## 프로젝트 개요

Arduino 기반 드론이 사전 지정된 직사각형 경로를 자율적으로 비행하여 시작점으로 정확하게 복귀하는 시스템입니다.

## 비행 경로

```
    4 ←─────── 3
    │     3m   ↑
2m  │          │ 2m
    ↓          │
    1 ───────→ 2
        3m
```

- 시작점 (Point 1): 이륙 및 착륙 위치
- Point 2: 우측으로 3m 이동
- Point 3: 전진 2m 이동
- Point 4: 좌측으로 3m 이동
- Point 1: 후진 2m로 시작점 복귀

## 주요 사양

### 비행 파라미터
- **순항 고도**: 120cm
- **비행 속도**: 최적화된 속도로 빠른 비행
- **호버링 시간**: 각 지점에서 0.5초

### 비행 시퀀스
1. **이륙 (2.5초)**: 부드러운 단계적 상승
2. **경로 비행**: 4개 지점을 순차적으로 방문
3. **착륙 (2.5초)**: 시작점에서 안전한 하강

## 하드웨어 구성

- Arduino Uno
- 드론 2호 (BLE 통신)
- HC-06 블루투스 모듈 (A0: RX, A1: TX)
- 제어 버튼
  - Pin 5: 시작 버튼
  - Pin 9: 비상정지 버튼

## 사용 방법

### 1. 준비
1. Arduino에 코드 업로드
2. 드론과 블루투스 페어링 확인
3. 드론을 시작 지점 (Point 1)에 위치
4. 시리얼 모니터 연결 (9600 baud)

### 2. 비행 시작
1. Pin 5 버튼을 눌러 자율비행 시작
2. 드론이 자동으로 이륙하여 경로 비행 수행
3. 시작점 복귀 후 자동 착륙

### 3. 비상 상황
- Pin 9 버튼을 누르면 즉시 비상정지 및 착륙

## 코드 구조

### 주요 함수

#### 이동 제어 함수
- `moveRight()`: 우측 이동 (1→2)
- `moveForward()`: 전진 (2→3)
- `moveLeft()`: 좌측 이동 (3→4)
- `moveBackward()`: 후진 (4→1)
- `hoverDrone()`: 호버링

#### 비행 단계 함수
- `takeoff()`: 단계적 이륙 시퀀스
- `land()`: 단계적 착륙 시퀀스
- `emergencyStop()`: 비상정지

#### 상태 머신
- `updateFlightPhase()`: 비행 단계 자동 전환

### 비행 단계 (FlightPhase)
```
IDLE → TAKEOFF → HOVER_START
  → MOVE_TO_P2 → HOVER_P2
  → MOVE_TO_P3 → HOVER_P3
  → MOVE_TO_P4 → HOVER_P4
  → MOVE_TO_P1 → HOVER_FINAL
  → LANDING → COMPLETED
```

## 성능 최적화

### 속도 최적화
- 각 구간별 최적화된 이동 시간
- 최소한의 호버링으로 빠른 경로 완주

### 정확도 최적화
- 시간 기반 경로 제어로 일관된 비행
- 부드러운 이착륙으로 위치 정확도 향상
- 각 지점에서 호버링으로 위치 보정

## 평가 기준 대응

### 1. 정확도
- 시작점 복귀 정확도: 시간 기반 제어로 일관된 경로
- 고도 유지: 120cm 일정 유지

### 2. 속도
- 총 비행 시간: 약 15초 (이착륙 제외)
- 최적화된 이동 속도와 최소 호버링

## 튜닝 가이드

실제 비행 테스트 후 다음 값들을 조정할 수 있습니다:

```cpp
// 비행 속도 조정
const int FLIGHT_SPEED = 100;  // 50~150 범위에서 조정

// 이동 시간 조정 (실제 거리와 속도에 맞춰 미세조정)
const unsigned long TIME_TO_P2 = 3500;  // 3m 이동
const unsigned long TIME_TO_P3 = 2500;  // 2m 이동
const unsigned long TIME_TO_P4 = 3500;  // 3m 이동
const unsigned long TIME_TO_P1 = 2500;  // 2m 이동

// 고도 조정
const int CRUISE_ALTITUDE = 120;  // 120cm
```

## 시리얼 모니터 출력

비행 중 다음과 같은 상태 메시지가 출력됩니다:

```
Test Started!
Flight Started!
Takeoff Complete - Hovering at Start
Moving to Point 2 (Right)
Reached Point 2 - Hovering
Moving to Point 3 (Forward)
Reached Point 3 - Hovering
Moving to Point 4 (Left)
Reached Point 4 - Hovering
Returning to Point 1 (Backward)
Returned to Start - Final Hovering
Starting Landing
Flight Completed!
```

## 라이선스

교육용 프로젝트
