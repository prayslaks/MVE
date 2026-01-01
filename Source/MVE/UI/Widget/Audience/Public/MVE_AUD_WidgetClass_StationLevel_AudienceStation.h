
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_StationLevel_AudienceStation.generated.h"


class UButton;
class UMVE_AUD_WidgetClass_ConcertList;
class UMVE_AUD_WidgetClass_SearchConcert;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_StationLevel_AudienceStation : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CustomButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ThrowCustomButton;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_AUD_WidgetClass_SearchConcert> SearchConcertWidget;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UMVE_AUD_WidgetClass_ConcertList> ConcertListWidget;
	
private:

	UFUNCTION()
	void OnCustomButtonClicked();

	UFUNCTION()
	void OnThrowCustomButtonClicked();
};
