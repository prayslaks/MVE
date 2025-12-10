
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_AUD_GM_AudienceCustomTest.generated.h"


UCLASS()
class MVE_API AMVE_AUD_GM_AudienceCustomTest : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
