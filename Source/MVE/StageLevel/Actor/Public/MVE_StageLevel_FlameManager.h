// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVE_StageLevel_SequenceManagerBase.h"
#include "MVE_StageLevel_FlameManager.generated.h"

UCLASS()
class MVE_API AMVE_StageLevel_FlameManager : public AMVE_StageLevel_SequenceManagerBase
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_FlameManager();
	
	virtual void BeginPlay() override;

	virtual void ExecuteSequenceNumber(int32 SequenceNumber, float DelayBetweenOrder) override;
	
	virtual void ExecuteSequenceActorByOrder() override;
};
