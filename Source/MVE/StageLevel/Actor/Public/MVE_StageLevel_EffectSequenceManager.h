
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageLevel/Data/MVE_EffectSequenceData.h"
#include "MVE_StageLevel_EffectSequenceManager.generated.h"

class AMVE_StageLevel_SpotlightManager;
class AMVE_StageLevel_PerformanceManager;

/**
 * Effect Sequence Manager
 * AI가 분석한 음악 타임라인에 따라 이펙트를 자동 재생하는 매니저
 * - 음악 재생 시간을 1/10초 단위로 추적
 * - TimeStamp에 도달하면 GameplayTag에 따라 SpotlightManager 또는 PerformanceManager 호출
 */
UCLASS()
class MVE_API AMVE_StageLevel_EffectSequenceManager : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_EffectSequenceManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/**
	 * AI 분석 결과를 등록
	 * @param SequenceData TimeStamp와 AssetID 배열
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Effect Sequence")
	void SetSequenceData(const TArray<FEffectSequenceData>& SequenceData);

	/**
	 * 시퀀스 재생 시작
	 * @param bFromBeginning true면 처음부터, false면 현재 시간부터
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Effect Sequence")
	void StartSequence(bool bFromBeginning = true);

	/**
	 * 시퀀스 재생 중지
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Effect Sequence")
	void StopSequence();

	/**
	 * 시퀀스 초기화 (처음으로 되돌림)
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Effect Sequence")
	void ResetSequence();

	/**
	 * 특정 시점으로 이동 (슬라이더 드래그 시 사용)
	 * @param TimeStamp 이동할 시점 (1/10초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Effect Sequence")
	void SeekToTimeStamp(int32 TimeStamp);

	/**
	 * 현재 재생 시간 가져오기 (1/10초 단위)
	 */
	UFUNCTION(BlueprintPure, Category = "MVE|Effect Sequence")
	int32 GetCurrentTimeStamp() const { return CurrentTimeStamp; }

	/**
	 * 재생 중인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "MVE|Effect Sequence")
	bool IsPlaying() const { return bIsPlaying; }

private:
	/**
	 * 타임라인 업데이트 (1/10초마다 호출)
	 * 현재 시간에 해당하는 이펙트가 있으면 실행
	 */
	void UpdateTimeline(float DeltaTime);

	/**
	 * 특정 TimeStamp의 이펙트 실행
	 * GameplayTag에 따라 SpotlightManager 또는 PerformanceManager 호출
	 */
	void ExecuteEffectAtTimeStamp(const FEffectSequenceData& Data);

	/**
	 * Spotlight AssetID를 SequenceNumber로 변환
	 * @param AssetID VFX.Spotlight.* 형식의 GameplayTag
	 * @return SequenceNumber (0~4), 변환 실패 시 -1
	 */
	int32 GetSpotlightSequenceNumber(const FGameplayTag& AssetID) const;

	/**
	 * 레벨에서 SpotlightManager와 PerformanceManager 찾기
	 */
	void FindManagers();

private:
	/** AI 분석 결과 (TimeStamp, AssetID) 배열 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MVE|Effect Sequence")
	TArray<FEffectSequenceData> EffectSequenceDataArray;

	/** 현재 재생 시간 (1/10초 단위) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MVE|Effect Sequence")
	int32 CurrentTimeStamp;

	/** 다음 실행할 이펙트의 인덱스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MVE|Effect Sequence")
	int32 NextEffectIndex;

	/** 재생 중 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MVE|Effect Sequence")
	bool bIsPlaying;

	/** 누적 시간 (DeltaTime 합산용) */
	float AccumulatedTime;

	/** SpotlightManager 레퍼런스 */
	UPROPERTY()
	AMVE_StageLevel_SpotlightManager* SpotlightManager;

	/** PerformanceManager 레퍼런스 */
	//UPROPERTY()
	//AMVE_StageLevel_PerformanceManager* PerformanceManager;
};
