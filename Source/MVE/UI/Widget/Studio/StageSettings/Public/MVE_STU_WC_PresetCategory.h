
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MVE_STU_PresetTypes.h"
#include "MVE_STU_WC_PresetCategory.generated.h"

class UMVE_STU_WC_PresetCategoryButton;
class UButton;
class UVerticalBox;

UCLASS()
class MVE_API UMVE_STU_WC_PresetCategory : public UUserWidget
{
	GENERATED_BODY()

public:
	// 카테고리 선택 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Preset")
	FOnCategorySelected OnCategorySelected;

protected:
	virtual void NativeConstruct() override;

	// 카테고리 버튼 컨테이너
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* CategoryButtonContainer;

	// 카테고리 버튼 클래스
	UPROPERTY(EditAnywhere, Category = "Preset")
	TSubclassOf<UUserWidget> CategoryButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Color")
	FLinearColor SelectedButtonColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Color")
	FLinearColor UnselectedButtonColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

private:
	// 카테고리 버튼들
	UPROPERTY()
	TMap<EPresetCategory, UMVE_STU_WC_PresetCategoryButton*> CategoryButtons;

	// 현재 선택된 카테고리 
	EPresetCategory CurrentCategory;

	// 카테고리 버튼 생성
	void CreateCategoryButtons();

	// 카테고리 버튼 클릭 처리
	UFUNCTION()
	void OnCategoryButtonClicked(EPresetCategory InCategory);

	// 선택된 카테고리 하이라이트
	void SetSelectedCategory(EPresetCategory InCategory);

	// 버튼 스타일 업데이트
	void UpdateButtonStyle(UMVE_STU_WC_PresetCategoryButton* ButtonWidget, bool bIsSelected);
};
