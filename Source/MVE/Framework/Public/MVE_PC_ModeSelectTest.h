
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MVE_PC_ModeSelectTest.generated.h"

UCLASS()
class MVE_API AMVE_PC_ModeSelectTest : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
