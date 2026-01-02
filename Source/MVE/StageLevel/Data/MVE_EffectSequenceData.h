// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVE_EffectSequenceData.generated.h"

/**
 * Effect Category
 * 이펙트의 카테고리 (Spotlight는 독립적, Performance는 Flame/Fanfare 중 하나만)
 */
UENUM(BlueprintType)
enum class EEffectCategory : uint8
{
	Spotlight   UMETA(DisplayName = "Spotlight"),
	Performance UMETA(DisplayName = "Performance")  // Flame or Fanfare
};

/**
 * Effect Sequence Data
 * AI가 분석한 음악 타임라인에 따라 재생할 이펙트 정보
 */
USTRUCT(BlueprintType)
struct MVE_API FEffectSequenceData
{
	GENERATED_BODY()

	/**
	 * 이펙트가 재생될 시점 (1/10초 단위)
	 * 예: 2분 31초 = 151초 = 1510
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Sequence")
	int32 TimeStamp;

	/**
	 * 재생할 이펙트의 GameplayTag
	 * 예: VFX.Spotlight.FastSpeed, VFX.Flame.VeryFastSizeAndVeryFastSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Sequence")
	FGameplayTag AssetID;

	/**
	 * 이펙트 카테고리
	 * Spotlight: 독립적으로 재생
	 * Performance: Flame 또는 Fanfare (동시 재생 불가)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Sequence")
	EEffectCategory Category;

	FEffectSequenceData()
		: TimeStamp(0)
		, AssetID(FGameplayTag())
		, Category(EEffectCategory::Spotlight)
	{
	}

	FEffectSequenceData(int32 InTimeStamp, FGameplayTag InAssetID, EEffectCategory InCategory)
		: TimeStamp(InTimeStamp)
		, AssetID(InAssetID)
		, Category(InCategory)
	{
	}
};

/**
 * Effect Sequence Data Array Wrapper
 * TMap에 TArray를 값으로 사용하기 위한 래퍼 구조체
 */
USTRUCT(BlueprintType)
struct MVE_API FEffectSequenceDataArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Sequence")
	TArray<FEffectSequenceData> SequenceData;

	FEffectSequenceDataArray()
	{
	}

	FEffectSequenceDataArray(const TArray<FEffectSequenceData>& InSequenceData)
		: SequenceData(InSequenceData)
	{
	}
};
