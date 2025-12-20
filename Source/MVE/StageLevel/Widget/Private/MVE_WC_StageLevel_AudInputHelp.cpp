#include "StageLevel/Widget/Public/MVE_WC_StageLevel_AudInputHelp.h"

#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/SlateWrapperTypes.h"

void UMVE_WC_StageLevel_AudInputHelp::SetInputHelpState(const EAudienceInputHelpState NewState)
{
	// 이미 동일한 상태
	if (CurrentState == NewState)
	{
		return;
	}

	// 새로운 상태 할당
	CurrentState = NewState;

	// 비가시
	if (FKeyHelpBox)
	{
		FKeyHelpBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MouseRightButtonHelpBox)
	{
		MouseRightButtonHelpBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MouseLeftButtonHelpBox)
	{
		MouseLeftButtonHelpBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 현재 상태에 따라 가시성 조작
	switch (CurrentState)
	{
	case EAudienceInputHelpState::Selectable:
		{
			FKeyHelpBox->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	case EAudienceInputHelpState::Executable:
		{
			FKeyHelpBox->SetVisibility(ESlateVisibility::Visible);
			MouseLeftButtonHelpBox->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	case EAudienceInputHelpState::Aimable:
		{
			FKeyHelpBox->SetVisibility(ESlateVisibility::Visible);
			MouseRightButtonHelpBox->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	default:
		{
			break;
		}
	}
}