
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_GenerateMesh.generated.h"

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

private:
	UFUNCTION()
	void OnSendPromptButtonClicked();
	
};
