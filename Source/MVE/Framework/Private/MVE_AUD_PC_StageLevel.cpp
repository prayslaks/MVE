
#include "../Public/MVE_AUD_PC_StageLevel.h"

#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

AMVE_AUD_PC_StageLevel::AMVE_AUD_PC_StageLevel()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMVE_AUD_PC_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인지 확인
	if (IsLocalPlayerController())
	{
		ShowAudienceUI();
	}
}

void AMVE_AUD_PC_StageLevel::ShowAudienceUI()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);

		// 입력 모드 설정 (UI + 게임)
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	
	
}
