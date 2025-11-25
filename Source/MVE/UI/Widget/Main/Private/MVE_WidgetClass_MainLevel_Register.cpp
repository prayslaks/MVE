
#include "../Public/MVE_WidgetClass_MainLevel_Register.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_MainLevel_Register::NativeConstruct()
{
	Super::NativeConstruct();

	if (SwitchLoginButton)
	{
		SwitchLoginButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnMoveLoginScreenButtonClicked);
	}

	if (TryRegisterButton)
	{
		TryRegisterButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnRegisterButtonClicked);
	}

	if (SendVerificationCodeButton)
	{
		SendVerificationCodeButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel_Register::OnUserPasswordEditableBoxCommitted()
{
}

void UMVE_WidgetClass_MainLevel_Register::OnVerificationPasswordEditableBoxCommitted()
{
}

void UMVE_WidgetClass_MainLevel_Register::ValidPasswordCondition()
{
}

void UMVE_WidgetClass_MainLevel_Register::OnSendVerificationCodeButtonClicked()
{
}

void UMVE_WidgetClass_MainLevel_Register::OnRegisterButtonClicked()
{
	// 회원가입 버튼 눌렀을 때
}

void UMVE_WidgetClass_MainLevel_Register::OnMoveLoginScreenButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Login);
	}
}
