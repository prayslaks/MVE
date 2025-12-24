
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WidgetClass_FinalCheckSettings.generated.h"

class UMVE_STD_WC_PlaylistBuilder;
class UEditableTextBox;
class UButton;



UCLASS()
class MVE_API UMVE_STD_WidgetClass_FinalCheckSettings : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartConcertButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMVE_STD_WC_PlaylistBuilder> PlaylistBuilderWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> RoomTitleEditableText;
	
private:
	UFUNCTION()
	void OnStartConcertButtonClicked();
};
