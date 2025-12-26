// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_SequenceManagerBase.generated.h"

class AMVE_StageLevel_SequenceActorBase;

USTRUCT(BlueprintType)
struct FSequenceActorGroup
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AMVE_StageLevel_SequenceActorBase*> SequenceActors;
};

UCLASS()
class MVE_API AMVE_StageLevel_SequenceManagerBase : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_SequenceManagerBase();
	
	UFUNCTION(BlueprintCallable, Category = "MVE|SequenceActor")
	virtual void ExecuteSequenceNumber(int32 SequenceNumber, float DelayBetweenOrder);
	
	UFUNCTION(BlueprintCallable, Category = "MVE|SequenceActor")
	virtual void ExecuteSequenceActorByOrder();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> TargetSequenceActorClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	TMap<int32, FSequenceActorGroup> SequenceActorsBySequenceOrder;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	TArray<int32> SequenceOrders;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	int32 CurrentSequenceOrderIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	int32 CurrentSequenceNumber;
	
	FTimerHandle ExecutionDelayTimerHandle;
};