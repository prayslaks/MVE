# Effect Sequence System 설계 문서

## 개요

음악 메타데이터를 AI가 분석하여 적절한 타이밍에 무대 이펙트(Spotlight, Flame, Fanfare)를 자동 재생하는 시스템.

---

## 시스템 구조

### 1. AI 분석 플로우

```
음악 메타데이터 + 에셋 리스트 → AI 서버
                                ↓
                    Array<{TimeStamp, AssetID}> 반환
                                ↓
                      EffectSequenceManager 등록
                                ↓
                    음악 재생 시 타임라인 추적 (1/10초 단위)
                                ↓
                TimeStamp 매칭 → SpotlightManager/PerformanceManager 호출

```

### 2. TimeStamp 형식

- **단위**: 1/10초 (int형)
- **예시**:
    - 2분 31초 = 151초 = **1510**
    - 0분 5.3초 = 5.3초 = **53**
    - 1분 0.7초 = 60.7초 = **607**

### 3. AI 응답 데이터 형식

```json
[
  {
    "TimeStamp": 100,
    "AssetID": "VFX.Spotlight.VerySlowSpeed"
  },
  {
    "TimeStamp": 1510,
    "AssetID": "VFX.Flame.FastSizeAndFastSpeed"
  },
  {
    "TimeStamp": 3245,
    "AssetID": "VFX.Fanfare.VeryHighSpawnRate"
  }
]

```

---

## GameplayTag 구조

### Category별 Manager 매핑

- `VFX.Spotlight.*` → **SpotlightManager**
- `VFX.Flame.*` → **PerformanceManager**
- `VFX.Fanfare.*` → **PerformanceManager**

### 에셋 리스트

### Spotlight (5개)

- `VFX.Spotlight.VerySlowSpeed` - 고요한 피아노 인트로, 침묵이 흐르는
- `VFX.Spotlight.SlowSpeed` - 서정적인 첫 소절, 따뜻하게 감싸는
- `VFX.Spotlight.NormalSpeed` - 경쾌한 팝 스타일, 리듬감 있는
- `VFX.Spotlight.FastSpeed` - 고조되는 댄스 비트, 긴장감이 팽팽한
- `VFX.Spotlight.VeryFastSpeed` - 락 장르 하이라이트, 폭발적인 아드레날린

### Flame (5개)

- `VFX.Flame.VerySmallSizeAndVerySlowSpeed` - 약한 촛불, 고요한 발라드 도입부
- `VFX.Flame.SmallSizeAndSlowSpeed` - 모닥불 크기, 편안한 미디엄 템포
- `VFX.Flame.NormalSmallSizeAndNormalSpeed` - 횃불, 힙합 비트와 스웨그
- `VFX.Flame.FastSizeAndFastSpeed` - 사람 키보다 큰 화염, 파워풀한 보컬
- `VFX.Flame.VeryFastSizeAndVeryFastSpeed` - 거대한 화염벽, 극강 클라이막스

### Fanfare (5개)

- `VFX.Fanfare.VeryLowSpawnRate` - 드물게 생성, 신비로운 인트로
- `VFX.Fanfare.LowSpawnRate` - 산들바람에 날리듯, 설레는 첫 소절
- `VFX.Fanfare.NormalSpawnRate` - 적당한 밀도, 즐거운 축제
- `VFX.Fanfare.HighSpawnRate` - 소나기처럼 쏟아짐, 폭발적인 하이라이트
- `VFX.Fanfare.VeryHighSpawnRate` - 폭풍우처럼 쏟아짐, 그랜드 피날레

---

## 기존 Manager 구조 (LJW 구현)

### SpotlightManager

- **역할**: Spotlight 에셋 5개만 관리/재생
- **SequenceNumber**: 에셋 인덱스 (0~4)
- **SequenceOrder**: 같은 에셋을 시간차 두고 실행하기 위한 순서
- **BeginPlay**: 레벨의 모든 Spotlight를 SequenceOrder별로 그룹핑
- **ExecuteSequence(SequenceNumber, DelayBetweenOrder)**: 시퀀스 실행

### PerformanceManager

- **역할**: Flame, Fanfare 에셋 관리/재생
- SpotlightManager와 유사한 구조

---

## EffectSequenceManager 설계

### 역할

- AI 결과(TimeStamp, GameplayTag) 저장
- 음악 타임라인 추적 (1/10초 단위 타이머)
- TimeStamp 매칭 시 적절한 Manager 호출

### 주요 기능

1. **SetSequenceData(Array<FEffectSequenceData>)** - AI 결과 등록
2. **StartSequence()** - 타임라인 시작
3. **StopSequence()** - 타임라인 정지
4. **ResetSequence()** - 초기화
5. **Tick (1/10초)** - 현재 시간 체크 → TimeStamp 매칭 → Manager 호출

### 데이터 구조체

```cpp
USTRUCT(BlueprintType)
struct FEffectSequenceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 TimeStamp; // 1/10초 단위

    UPROPERTY(BlueprintReadWrite)
    FGameplayTag AssetID; // VFX.Spotlight.*, VFX.Flame.*, VFX.Fanfare.*
};

```

### Manager 호출 로직

```cpp
void ExecuteEffectAtTimeStamp(const FEffectSequenceData& Data)
{
    FString Category = Data.AssetID.ToString().Left(13); // "VFX.Spotlight" or "VFX.Flame" or "VFX.Fanfare"

    if (Category.Contains("Spotlight"))
    {
        SpotlightManager->ExecuteByTag(Data.AssetID);
    }
    else // Flame or Fanfare
    {
        PerformanceManager->ExecuteByTag(Data.AssetID);
    }
}

```

---

## 프리뷰 위젯 설계

### UI 구성

```
Canvas Panel
├─ Slider (음악 타임라인)
│  └─ Fill (진행 바)
├─ Canvas Panel (아이콘 오버레이)
│  ├─ Image (TimeStamp 위치 - 💡/🔥/🎉)
│  └─ ...
└─ Button (재생/일시정지)

```

### 주요 기능

1. **음악 재생 슬라이더**
    - 현재 재생 시간에 따라 자동 업데이트
    - 사용자 드래그로 특정 시점 이동 (옵션)
2. **TimeStamp 아이콘 표시**
    - AI 결과 수신 시 각 TimeStamp 위치에 아이콘 생성
    - 아이콘 종류: Spotlight(💡), Flame(🔥), Fanfare(🎉)
    - 위치 계산: `X = (TimeStamp / TotalDuration) * SliderWidth`
3. **시각적 피드백**
    - 현재 재생 중인 이펙트 하이라이트
    - 호버 툴팁: "2:31 - Flame FastSpeed"
    - 클릭 점프: 아이콘 클릭 → 해당 시점으로 이동

### 플로우

```
PlaylistBuilder에서 음악 선택
         ↓
AI 서버에 메타데이터 전송 (GenAISenderReceiver)
         ↓
AI 결과 수신 → EffectSequenceManager에 등록
         ↓
프리뷰 위젯에 아이콘 표시 (TimeStamp 위치)
         ↓
음악 재생 시작 → EffectSequenceManager.StartSequence()
         ↓
타임라인 추적 → 슬라이더 업데이트 + 이펙트 재생

```

---

## 구현 순서

1. **데이터 구조체** - `FEffectSequenceData` 정의
2. **EffectSequenceManager** - 타임라인 추적 + Manager 호출 로직
3. **프리뷰 위젯** - UI + 슬라이더 + 아이콘 표시
4. **AI 연동** - GenAISenderReceiver 통합

---

## 참고사항

- **기존 시스템 재사용**: SpotlightManager, PerformanceManager는 수정하지 않고 호출만
- **StageLevel 배치**: 모든 이펙트 액터는 StageLevel에 배치됨
- **프리뷰와 실제 공연**: EffectSequenceManager는 둘 다 사용 가능하도록 설계
- **1/10초 정밀도**: 음악 재생 시간 추적은 0.1초 단위 타이머 사용

---

**작성일**: 2025-12-26
**작성자**: Claude Code
**프로젝트**: MVE (Virtual Idol Studio)