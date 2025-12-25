// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_SequenceActorBase.generated.h"

UCLASS()
class MVE_API AMVE_StageLevel_SequenceActorBase : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_SequenceActorBase();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SequenceOrder;
	
	UFUNCTION(BlueprintImplementableEvent)
	void ExecuteSequence(int32 SequenceNumber);
};
