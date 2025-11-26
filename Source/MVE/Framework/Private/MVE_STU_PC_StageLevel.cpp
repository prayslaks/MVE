
#include "../Public/MVE_STU_PC_StageLevel.h"

#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

AMVE_STU_PC_StageLevel::AMVE_STU_PC_StageLevel()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMVE_STU_PC_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인지 확인
	if (IsLocalPlayerController())
	{
		ShowStudioUI();
	}
}

void AMVE_STU_PC_StageLevel::ShowStudioUI()
{
	if (!StudioUIClass) return;

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::StudioBroadcast);

		// 입력 모드 설정 (UI + 게임)
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}
