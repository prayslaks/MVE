
#include "../Public/MVE_WidgetClass_MainLevel_Credit.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_MainLevel_Credit::NativeConstruct()
{
	Super::NativeConstruct();

	if (MoveMainButton)
	{
		MoveMainButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Credit::OnMoveMainButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel_Credit::OnMoveMainButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
}
