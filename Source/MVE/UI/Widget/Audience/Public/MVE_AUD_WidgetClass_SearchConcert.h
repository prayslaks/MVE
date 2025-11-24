
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_SearchConcert.generated.h"

class UEditableTextBox;
class UButton;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_SearchConcert : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* ConcertSearchEditableTextBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SearchButton;
	
};
