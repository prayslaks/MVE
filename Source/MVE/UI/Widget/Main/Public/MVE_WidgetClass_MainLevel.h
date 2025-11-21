
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// 버튼 콜백
	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION()
	void OnReturnDesktopButtonClicked();

	UFUNCTION()
	void OnCreditButtonClicked();

	// UI 바인딩
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnDesktopButton;

	UPROPERTY(meta = (BindWidget))
	UButton* LoginButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreditButton;
};
