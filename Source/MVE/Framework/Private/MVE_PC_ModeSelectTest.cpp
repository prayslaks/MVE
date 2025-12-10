
#include "../Public/MVE_PC_ModeSelectTest.h"

#include "UIManagerSubsystem.h"

void AMVE_PC_ModeSelectTest::BeginPlay()
{
	Super::BeginPlay();

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::ModeSelect);
	}
}
