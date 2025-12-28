// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_StageLevel_EffectSequenceManager.h"
#include "MVE_StageLevel_SpotlightManager.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_EffectSequenceManager::AMVE_StageLevel_EffectSequenceManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f; // 매 프레임 Tick

	CurrentTimeStamp = 0;
	NextEffectIndex = 0;
	bIsPlaying = false;
	AccumulatedTime = 0.0f;
	SpotlightManager = nullptr;
	//PerformanceManager = nullptr;
}

void AMVE_StageLevel_EffectSequenceManager::BeginPlay()
{
	Super::BeginPlay();

	FindManagers();
}

void AMVE_StageLevel_EffectSequenceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPlaying)
	{
		UpdateTimeline(DeltaTime);
	}
}

void AMVE_StageLevel_EffectSequenceManager::SetSequenceData(const TArray<FEffectSequenceData>& SequenceData)
{
	EffectSequenceDataArray = SequenceData;

	// TimeStamp 기준으로 정렬 (오름차순)
	EffectSequenceDataArray.Sort([](const FEffectSequenceData& A, const FEffectSequenceData& B)
	{
		return A.TimeStamp < B.TimeStamp;
	});

	ResetSequence();

	PRINTLOG(TEXT("Effect Sequence Data 등록 완료: %d개"), EffectSequenceDataArray.Num());
}

void AMVE_StageLevel_EffectSequenceManager::StartSequence(bool bFromBeginning)
{
	if (bFromBeginning)
	{
		ResetSequence();
	}

	bIsPlaying = true;
	AccumulatedTime = 0.0f;

	PRINTLOG(TEXT("Effect Sequence 시작 - TimeStamp: %d"), CurrentTimeStamp);
}

void AMVE_StageLevel_EffectSequenceManager::StopSequence()
{
	bIsPlaying = false;

	PRINTLOG(TEXT("Effect Sequence 중지 - TimeStamp: %d"), CurrentTimeStamp);
}

void AMVE_StageLevel_EffectSequenceManager::ResetSequence()
{
	CurrentTimeStamp = 0;
	NextEffectIndex = 0;
	AccumulatedTime = 0.0f;

	PRINTLOG(TEXT("Effect Sequence 초기화"));
}

void AMVE_StageLevel_EffectSequenceManager::SeekToTimeStamp(int32 TimeStamp)
{
	CurrentTimeStamp = TimeStamp;
	AccumulatedTime = 0.0f;

	// NextEffectIndex 재계산: TimeStamp보다 큰 첫 번째 인덱스 찾기
	NextEffectIndex = 0;
	for (int32 i = 0; i < EffectSequenceDataArray.Num(); ++i)
	{
		if (EffectSequenceDataArray[i].TimeStamp > CurrentTimeStamp)
		{
			NextEffectIndex = i;
			break;
		}
		NextEffectIndex = i + 1; // 마지막까지 다 지나간 경우
	}

	PRINTLOG(TEXT("TimeStamp로 이동: %d, NextEffectIndex: %d"), TimeStamp, NextEffectIndex);
}

void AMVE_StageLevel_EffectSequenceManager::UpdateTimeline(float DeltaTime)
{
	AccumulatedTime += DeltaTime;

	// 0.1초(1/10초)마다 TimeStamp 증가
	const float TimeStampInterval = 0.1f;

	while (AccumulatedTime >= TimeStampInterval)
	{
		AccumulatedTime -= TimeStampInterval;
		CurrentTimeStamp++;

		// 현재 TimeStamp에 해당하는 이펙트가 있는지 체크
		while (NextEffectIndex < EffectSequenceDataArray.Num())
		{
			const FEffectSequenceData& EffectData = EffectSequenceDataArray[NextEffectIndex];

			if (EffectData.TimeStamp == CurrentTimeStamp)
			{
				// 이펙트 실행
				ExecuteEffectAtTimeStamp(EffectData);
				NextEffectIndex++;
			}
			else if (EffectData.TimeStamp > CurrentTimeStamp)
			{
				// 아직 시간이 안 됨
				break;
			}
			else
			{
				// 이미 지나간 이펙트 (건너뜀)
				NextEffectIndex++;
			}
		}
	}
}

void AMVE_StageLevel_EffectSequenceManager::ExecuteEffectAtTimeStamp(const FEffectSequenceData& Data)
{
	if (!Data.AssetID.IsValid())
	{
		PRINTLOG(TEXT("유효하지 않은 AssetID"));
		return;
	}

	FString TagString = Data.AssetID.ToString();

	PRINTLOG(TEXT("🎬 Effect 실행 - TimeStamp: %d, AssetID: %s"), Data.TimeStamp, *TagString);

	// GameplayTag를 파싱해서 카테고리 확인
	// 예: "VFX.Spotlight.FastSpeed" → "VFX.Spotlight"
	// 예: "VFX.Flame.VeryFastSizeAndVeryFastSpeed" → "VFX.Flame"
	// 예: "VFX.Fanfare.HighSpawnRate" → "VFX.Fanfare"

	if (TagString.StartsWith(TEXT("VFX.Spotlight")))
	{
		// Spotlight 이펙트
		if (SpotlightManager)
		{
			int32 SequenceNumber = GetSpotlightSequenceNumber(Data.AssetID);
			if (SequenceNumber >= 0)
			{
				float DelayBetweenOrder = 0.0f; // 동시 실행
				SpotlightManager->ExecuteSequenceNumber(SequenceNumber, DelayBetweenOrder);
				PRINTLOG(TEXT("✅ SpotlightManager 실행 - SequenceNumber: %d, AssetID: %s"), SequenceNumber, *TagString);
			}
			else
			{
				PRINTLOG(TEXT("⚠️ 유효하지 않은 Spotlight AssetID: %s"), *TagString);
			}
		}
		else
		{
			PRINTLOG(TEXT("❌ SpotlightManager를 찾을 수 없습니다"));
		}
	}
	else if (TagString.StartsWith(TEXT("VFX.Flame")) || TagString.StartsWith(TEXT("VFX.Fanfare")))
	{
		/*
		// Flame 또는 Fanfare 이펙트
		if (PerformanceManager)
		{
			// TODO: PerformanceManager에 GameplayTag 기반 실행 함수 추가 필요
			// PerformanceManager->ExecuteByTag(Data.AssetID);
			PRINTLOG(TEXT("PerformanceManager 호출: %s"), *TagString);
		}
		else
		{
			PRINTLOG(TEXT("PerformanceManager를 찾을 수 없습니다"));
		}
		*/
		PRINTLOG(TEXT("🔜 PerformanceManager 연동 예정 - AssetID: %s"), *TagString);
	}
	else
	{
		PRINTLOG(TEXT("❓ 알 수 없는 카테고리의 AssetID: %s"), *TagString);
	}
}

int32 AMVE_StageLevel_EffectSequenceManager::GetSpotlightSequenceNumber(const FGameplayTag& AssetID) const
{
	FString TagString = AssetID.ToString();

	// VFX.Spotlight.VerySlowSpeed → 0
	if (TagString == TEXT("VFX.Spotlight.VerySlowSpeed"))
	{
		return 0;
	}
	// VFX.Spotlight.SlowSpeed → 1
	else if (TagString == TEXT("VFX.Spotlight.SlowSpeed"))
	{
		return 1;
	}
	// VFX.Spotlight.NormalSpeed → 2
	else if (TagString == TEXT("VFX.Spotlight.NormalSpeed"))
	{
		return 2;
	}
	// VFX.Spotlight.FastSpeed → 3
	else if (TagString == TEXT("VFX.Spotlight.FastSpeed"))
	{
		return 3;
	}
	// VFX.Spotlight.VeryFastSpeed → 4
	else if (TagString == TEXT("VFX.Spotlight.VeryFastSpeed"))
	{
		return 4;
	}
	else
	{
		// 변환 실패
		return -1;
	}
}

void AMVE_StageLevel_EffectSequenceManager::FindManagers()
{
	// SpotlightManager 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_SpotlightManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		SpotlightManager = Cast<AMVE_StageLevel_SpotlightManager>(FoundActors[0]);
		PRINTLOG(TEXT("SpotlightManager 찾음"));
	}
	else
	{
		PRINTLOG(TEXT("SpotlightManager를 찾을 수 없습니다"));
	}

	// PerformanceManager 찾기
	// TODO: PerformanceManager 클래스가 구현되면 주석 해제
	/*
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_PerformanceManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		PerformanceManager = Cast<AMVE_StageLevel_PerformanceManager>(FoundActors[0]);
		PRINTLOG(TEXT("PerformanceManager 찾음"));
	}
	else
	{
		PRINTLOG(TEXT("PerformanceManager를 찾을 수 없습니다"));
	}
	*/
}
