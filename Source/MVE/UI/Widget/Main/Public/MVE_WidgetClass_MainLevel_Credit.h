
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_Credit.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_Credit : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GoMainButton;
};
