#include "MVE_GM_PreviewStageLevel.h"
#include "UIManagerSubsystem.h"
#include "MVE.h"

AMVE_GM_PreviewStageLevel::AMVE_GM_PreviewStageLevel()
{
	// GameMode 기본 설정
}

void AMVE_GM_PreviewStageLevel::BeginPlay()
{
	Super::BeginPlay();

	// FinalCheckSettings UI 표시
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::Studio_FinalCheckSetting);
		PRINTLOG(TEXT("PreviewStageLevel 시작 - FinalCheckSettings UI 표시"));
	}
	else
	{
		PRINTLOG(TEXT("UIManagerSubsystem을 찾을 수 없습니다!"));
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->bShowMouseCursor = true;
	}
	
}
