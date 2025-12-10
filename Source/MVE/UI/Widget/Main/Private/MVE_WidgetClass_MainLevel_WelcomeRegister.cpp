// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_WidgetClass_MainLevel_WelcomeRegister.h"

#include "MVE_WidgetClass_MainLevel_Register.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_MainLevel_WelcomeRegister::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 로그인 이동 바인딩
	if (MoveLoginButton)
	{
		MoveLoginButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_WelcomeRegister::OnMoveLoginButtonClicked);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMVE_WidgetClass_MainLevel_WelcomeRegister::OnMoveLoginButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Login);
	}
}
