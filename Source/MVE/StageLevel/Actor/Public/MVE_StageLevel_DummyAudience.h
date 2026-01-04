// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_DummyAudience.generated.h"

UCLASS()
class MVE_API AMVE_StageLevel_DummyAudience : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_DummyAudience();
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsDancing(const bool Value) { bIsDancing = Value; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDancing;
};