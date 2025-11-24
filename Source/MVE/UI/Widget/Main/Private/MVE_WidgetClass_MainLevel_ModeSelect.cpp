
#include "../Public/MVE_WidgetClass_MainLevel_ModeSelect.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_MainLevel_ModeSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (MoveStudioButton)
	{
		MoveStudioButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveStudioButtonClicked);
	}

	if (MoveAudienceButton)
	{
		MoveAudienceButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveAudienceButtonClicked);
	}

	if (MoveMainButton)
	{
		MoveMainButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveMainButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveStudioButtonClicked()
{
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveAudienceButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);
	}
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveMainButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
}
