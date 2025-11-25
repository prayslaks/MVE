
#include "../Public/MVE_WidgetClass_MainLevel_Login.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_MainLevel_Login::NativeConstruct()
{
	Super::NativeConstruct();

	if (MoveMainButton)
	{
		MoveMainButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnMoveMainMenuButtonClicked);
	}

	if (TryLoginButton)
	{
		TryLoginButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnLoginButtonClicked);
	}

	if (MoveRegisterButton)
	{
		MoveRegisterButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Login::OnRegisterButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnUserEmailEditableBoxCommitted()
{
	
}

void UMVE_WidgetClass_MainLevel_Login::OnUserPasswordEditableBoxCommitted()
{
}

bool UMVE_WidgetClass_MainLevel_Login::ValidLoginForm()
{
	return true;
}

void UMVE_WidgetClass_MainLevel_Login::OnLoginButtonClicked()
{
	// 로그인 처리하셈

	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::ModeSelect);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnRegisterButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Register);
	}
}

void UMVE_WidgetClass_MainLevel_Login::OnMoveMainMenuButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
}
