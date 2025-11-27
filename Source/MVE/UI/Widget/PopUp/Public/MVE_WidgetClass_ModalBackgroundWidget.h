
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_ModalBackgroundWidget.generated.h"


UCLASS()
class MVE_API UMVE_WidgetClass_ModalBackgroundWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBackgroundBlur> BackgroundBlur;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> DismissButton;

public:
	/**
	 * 블러 강도 설정
	 */
	UFUNCTION(BlueprintCallable)
	void SetBlurStrength(float Strength);

	/**
	 * 배경 어둡기 설정 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintCallable)
	void SetBackgroundDarkness(float Darkness);

	// 배경 클릭 시 팝업 닫기 이벤트
	DECLARE_MULTICAST_DELEGATE(FOnBackgroundClicked);
	FOnBackgroundClicked OnBackgroundClicked;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnDismissButtonClicked();
};
