
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel : public UUserWidget
{
	GENERATED_BODY()


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnDesktopButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CreditButton;
};
