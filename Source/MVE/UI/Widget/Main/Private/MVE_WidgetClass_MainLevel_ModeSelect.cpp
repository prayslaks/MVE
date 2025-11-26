
#include "../Public/MVE_WidgetClass_MainLevel_ModeSelect.h"

#include "MVE_GIS_SessionManager.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"

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
	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		FRoomInfo RoomInfo;
		RoomInfo.RoomTitle = FString(TEXT("오늘 점심은 편의점"));
		RoomInfo.MaxViewers = 10;

		// 현재 PlayerController를 전달하여 이 플레이어만 이동시킴
		APlayerController* PC = GetOwningPlayer();
		SessionManager->CreateSession(RoomInfo, PC);
	}
}

void UMVE_WidgetClass_MainLevel_ModeSelect::OnMoveAudienceButtonClicked()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);
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
