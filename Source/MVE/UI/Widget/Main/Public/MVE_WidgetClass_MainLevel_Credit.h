
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_Credit.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_Credit : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveMainButton;

	UFUNCTION()
	void OnMoveMainButtonClicked();
};
