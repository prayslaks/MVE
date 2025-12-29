
#include "../Public/MVE_STU_Button_TabButton.h"
#include "MVE.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Data/BroadcastSettingTypes.h"

void UMVE_STU_Button_TabButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Tab)
	{
		Btn_Tab->OnClicked.AddDynamic(this, &UMVE_STU_Button_TabButton::OnTabClicked);
	}
	
}

void UMVE_STU_Button_TabButton::SetupTab(const FBroadcastSettingTabInfo& TabInfo)
{
	TabType = TabInfo.TabType;
	
	if (Text_TabName)
	{
		Text_TabName->SetText(TabInfo.TabText);
	}

	UpdateVisuals();
}

void UMVE_STU_Button_TabButton::SetSelected(bool bInSelected)
{
	if (bIsSelected == bInSelected)
	{
		return;
	}

	bIsSelected = bInSelected;
	UpdateVisuals();
}

void UMVE_STU_Button_TabButton::OnTabClicked()
{
	PRINTLOG(TEXT("Tab clicked: %d"), (int32)TabType);
    
	if (OnTabButtonClicked.IsBound())
	{
		OnTabButtonClicked.Broadcast(TabType);
	}
}

void UMVE_STU_Button_TabButton::UpdateVisuals()
{
	if (!BackgroundBorder || !Btn_Tab || !Text_TabName)
	{
		return;
	}

	FLinearColor CurrentColor = bIsSelected ? SelectedColor : UnselectedColor;

	// Border 배경색 변경 - Brush를 새로 생성하여 설정
	FSlateBrush NewBrush;
	NewBrush.TintColor = FSlateColor(CurrentColor);
	BackgroundBorder->SetBrush(NewBrush);

	// 텍스트 색상 변경 (선택 시 흰색, 비선택 시 회색)
	//FSlateColor TextColor = bIsSelected ?
	//	FSlateColor(FLinearColor::White) :
	//	FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));

	//Text_TabName->SetColorAndOpacity(TextColor);
}
