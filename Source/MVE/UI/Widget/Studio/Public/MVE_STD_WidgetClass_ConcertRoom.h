
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WidgetClass_ConcertRoom.generated.h"

class UVerticalBox;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class MVE_API UMVE_STD_WidgetClass_ConcertRoom : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> RoomTitleEditableBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> ViewerListVerticalBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ViewerCountsTextBox;


};
