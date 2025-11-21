
#include "../Public/MVE_WidgetClass_MainLevel.h"

#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UMVE_WidgetClass_MainLevel::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 바인딩
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel::OnLoginButtonClicked);
	}
	if (CreditButton)
	{
		CreditButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel::OnCreditButtonClicked);
	}

	if (ReturnDesktopButton)
	{
		ReturnDesktopButton->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel::OnReturnDesktopButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel::OnLoginButtonClicked()
{
	PRINTLOG(TEXT("OnLoginButtonClicked Called"));
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Login);
	}
}

void UMVE_WidgetClass_MainLevel::OnReturnDesktopButtonClicked()
{
	PRINTLOG(TEXT("OnReturnDesktopButtonClicked Called"));
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

void UMVE_WidgetClass_MainLevel::OnCreditButtonClicked()
{
	PRINTLOG(TEXT("OnCreditButtonClicked Called"));
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Credit);
	}
}
