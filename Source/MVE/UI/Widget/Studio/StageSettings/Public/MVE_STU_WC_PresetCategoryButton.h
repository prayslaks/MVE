
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MVE_STU_PresetTypes.h"
#include "MVE_STU_WC_PresetCategoryButton.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategoryButtonClicked, EPresetCategory, Category);

UCLASS()
class MVE_API UMVE_STU_WC_PresetCategoryButton : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetCategory(EPresetCategory InCategory) { Category = InCategory; }
	
	FOnCategoryButtonClicked OnCategoryButtonClicked;

	void SetCategoryNameText(FText PresetName);

	UFUNCTION(BlueprintImplementableEvent)
	void SetButtonUnselected();

	UFUNCTION(BlueprintImplementableEvent)
	void SetButtonSelected();

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CategoryNameText;

	EPresetCategory Category;


private:
	UFUNCTION()
	void OnButtonClicked();

	
	
};
