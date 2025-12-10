
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_ConcertRoom.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_ConcertRoom : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> WaveButton;

	UFUNCTION()
	void OnWaveButtonClicked();
};
