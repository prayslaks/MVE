
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_DropdownOverlay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOverlayClicked);

UCLASS()
class MVE_API UMVE_WidgetClass_DropdownOverlay : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Overlay;

	UFUNCTION()
	void OnOverlayClicked();

public:
	// 외부에서 구독할 수 있는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnOverlayClicked OnOverlayClickedEvent;
};
