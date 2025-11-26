
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_GM_StageLevel.generated.h"

UCLASS()
class MVE_API AMVE_GM_StageLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMVE_GM_StageLevel();

protected:
	virtual void BeginPlay() override;
	
};
