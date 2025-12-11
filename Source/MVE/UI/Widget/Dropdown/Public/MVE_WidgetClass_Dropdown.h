
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_Dropdown.generated.h"

UCLASS()
class MVE_API UMVE_WidgetClass_Dropdown : public UUserWidget
{
	GENERATED_BODY()

public:
	// 드롭다운 위치 설정
	void SetDropdownPosition(const FVector2D& InPosition);

	// 드롭다운 닫기
	UFUNCTION(BlueprintCallable)
	void CloseDropdown();
};
