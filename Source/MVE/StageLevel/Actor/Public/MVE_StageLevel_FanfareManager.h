// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVE_StageLevel_SequenceManagerBase.h"
#include "MVE_StageLevel_FanfareManager.generated.h"

UCLASS()
class MVE_API AMVE_StageLevel_FanfareManager : public AMVE_StageLevel_SequenceManagerBase
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_FanfareManager();

	virtual void BeginPlay() override;
};
