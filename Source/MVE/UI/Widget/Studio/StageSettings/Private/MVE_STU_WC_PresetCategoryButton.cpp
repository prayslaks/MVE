
#include "../Public/MVE_STU_WC_PresetCategoryButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMVE_STU_WC_PresetCategoryButton::SetCategoryNameText(FText PresetName)
{
	CategoryNameText->SetText(PresetName);
}

void UMVE_STU_WC_PresetCategoryButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
		Button.Get()->OnClicked.AddDynamic(this, &UMVE_STU_WC_PresetCategoryButton::OnButtonClicked);
}

void UMVE_STU_WC_PresetCategoryButton::OnButtonClicked()
{
	OnCategoryButtonClicked.Broadcast(Category);
}
