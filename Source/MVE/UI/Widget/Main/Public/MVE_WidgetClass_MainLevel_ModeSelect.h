
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_ModeSelect.generated.h"

class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_ModeSelect : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveStudioButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveAudienceButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MoveMainButton;

	// 버튼 콜백함수

	UFUNCTION()
	void OnMoveStudioButtonClicked();

	UFUNCTION()
	void OnMoveAudienceButtonClicked();

	UFUNCTION()
	void OnMoveMainButtonClicked();
};
