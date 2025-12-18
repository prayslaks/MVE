
#include "../Public/MVE_PC_Master.h"

#include "MVE_WidgetClass_MainLevel.h"
#include "NetworkMessage.h"
#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

void AMVE_PC_Master::BeginPlay()
{
	Super::BeginPlay();

	// Input Mode, Mouse Cursor 보이기 설정
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI M;
	M.SetHideCursorDuringCapture(false);
	SetInputMode(M);

	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Intro);
	}
	
}
