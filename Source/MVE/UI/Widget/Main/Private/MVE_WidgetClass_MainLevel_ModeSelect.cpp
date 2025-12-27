
#include "../Public/MVE_WidgetClass_MainLevel_ModeSelect.h"

#include "MVE_GIS_SessionManager.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMVE_WidgetClass_MainLevel_ModeSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (MoveStudioButton)
	{
		MoveStudioButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveStudioButtonClicked);
	}

	if (MoveAudienceButton)
	{
		MoveAudienceButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveAudienceButtonClicked);
	}

	if (MoveMainButton)
	{
		MoveMainButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveMainButtonClicked);
	}
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveStudioButtonClicked()
{
	// PreviewStageLevel로 전환
	UGameplayStatics::OpenLevel(this, FName(TEXT("PreviewStageLevel")), true);
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveAudienceButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("CharacterCustomTestMap")), true);
	}
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveMainButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::Main);
	}
}
