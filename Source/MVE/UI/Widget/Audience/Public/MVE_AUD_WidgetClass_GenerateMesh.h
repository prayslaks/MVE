
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_GenerateMesh.generated.h"

class UTextBlock;
class UButton;
class UMultiLineEditableTextBox;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_GenerateMesh : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMultiLineEditableTextBox> PromptEditableBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SendPromptButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUserWidget> MeshPreviewWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> EnterConcertRoomButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> InputImageButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ImportedImageNameTextBlock;

private:
	UFUNCTION()
	void OnSendPromptButtonClicked();

	UFUNCTION()
	void OnEnterConcertRoomButtonClicked();

	UFUNCTION()
	void OnInputImageButtonClicked();
	
};
