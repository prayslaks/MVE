#pragma once

#include "CoreMinimal.h"
#include "MVE_StageLevel_SequenceManagerBase.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_SpotlightManager.generated.h"

class AMVE_StageLevel_SequenceActorBase;


UCLASS()
class MVE_API AMVE_StageLevel_SpotlightManager : public AMVE_StageLevel_SequenceManagerBase
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_SpotlightManager();
	
	virtual void BeginPlay() override;
	
	virtual void ExecuteSequenceNumber(int32 SequenceNumber, float DelayBetweenOrder) override;
	
	virtual void ExecuteSequenceActorByOrder() override;
};