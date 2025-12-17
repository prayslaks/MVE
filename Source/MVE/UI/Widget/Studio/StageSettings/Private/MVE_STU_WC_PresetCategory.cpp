
#include "../Public/MVE_STU_WC_PresetCategory.h"

#include "MVE.h"
#include "MVE_STU_WC_PresetCategoryButton.h"
#include "AnimNodes/AnimNode_RandomPlayer.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"

void UMVE_STU_WC_PresetCategory::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentCategory = EPresetCategory::Effect;
	CreateCategoryButtons();
}

void UMVE_STU_WC_PresetCategory::CreateCategoryButtons()
{
	if (!CategoryButtonContainer)
	{
		PRINTLOG(TEXT("CategoryButtonContainer is nullptr"));
		return;
	}

	// 기존 버튼 제거
	CategoryButtonContainer->ClearChildren();
	CategoryButtons.Empty();

	// 카테고리별 버튼 생성
	TArray<TPair<EPresetCategory, FText>> Categories = {
		{EPresetCategory::Effect, FText::FromString(TEXT("이펙트"))},
		{EPresetCategory::Lighting, FText::FromString(TEXT("조명"))},
		{EPresetCategory::CameraEffect, FText::FromString(TEXT("카메라 효과"))},
		{EPresetCategory::StageBackground, FText::FromString(TEXT("무대 배경"))}
	};

	for (int32 i = 0; i < Categories.Num(); ++i)
	{
		if (CategoryButtonClass)
		{
			UUserWidget* NewWidget = CreateWidget<UUserWidget>(this, CategoryButtonClass);
			UMVE_STU_WC_PresetCategoryButton* ButtonWidget = Cast<UMVE_STU_WC_PresetCategoryButton>(NewWidget);
			ButtonWidget->SetCategory(Categories[i].Key);
			ButtonWidget->SetCategoryNameText(Categories[i].Value);
			ButtonWidget->OnCategoryButtonClicked.AddDynamic(this, &UMVE_STU_WC_PresetCategory::OnCategoryButtonClicked);
			
			CategoryButtonContainer->AddChildToVerticalBox(ButtonWidget);
			CategoryButtons.Add(Categories[i].Key, ButtonWidget);
		}
	}

	// 첫 번째 카테고리 기본 선택
	if (CategoryButtons.Num() > 0)
	{
		SetSelectedCategory(EPresetCategory::Effect);
	}
}

void UMVE_STU_WC_PresetCategory::OnCategoryButtonClicked(EPresetCategory InCategory)
{
	EPresetCategory SelectedCategory = InCategory;
	
	// 카테고리 선택 델리게이트 발생
	SetSelectedCategory(InCategory);
	OnCategorySelected.Broadcast(SelectedCategory);

	PRINTLOG(TEXT("Category selected: %d"), InCategory);
}

void UMVE_STU_WC_PresetCategory::SetSelectedCategory(EPresetCategory InCategory)
{
	// 이전 선택 해제
	UpdateButtonStyle(CategoryButtons[CurrentCategory], false);

	// 새로운 선택 적용
	CurrentCategory = InCategory;
	UpdateButtonStyle(CategoryButtons[CurrentCategory], true);
}

void UMVE_STU_WC_PresetCategory::UpdateButtonStyle(UMVE_STU_WC_PresetCategoryButton* ButtonWidget, bool bIsSelected)
{
	if (!ButtonWidget)
	{
		return;
	}

	if (bIsSelected)
	{
		// 선택된 상태 스타일
		ButtonWidget->SetButtonSelected();
	}
	else
	{
		// 기본 상태 스타일
		ButtonWidget->SetButtonUnselected();
	}
}
