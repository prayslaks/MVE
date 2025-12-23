
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MVE_PC_Master.generated.h"

class UMVE_WidgetClass_MainLevel;
struct FLoginResponseData;

UCLASS()
class MVE_API AMVE_PC_Master : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVE_WidgetClass_MainLevel> MainWidgetClass;

private:
	// 자동 로그인 콜백
	UFUNCTION()
	void OnAutoLoginComplete(bool bSuccess, const FLoginResponseData& ResponseData, const FString& ErrorCode);
};
