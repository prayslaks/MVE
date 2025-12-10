
#include "../Public/MVE_STD_WidgetClass_FinalCheckSettings.h"
#include "MVE_GIS_SessionManager.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Data/RoomInfo.h"

void UMVE_STD_WidgetClass_FinalCheckSettings::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartConcertButton)
		StartConcertButton.Get()->OnClicked.AddDynamic(this, &UMVE_STD_WidgetClass_FinalCheckSettings::OnStartConcertButtonClicked);
}

void UMVE_STD_WidgetClass_FinalCheckSettings::OnStartConcertButtonClicked()
{
	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		FRoomInfo RoomInfo;
		FString RoomTitle = RoomTitleEditableText->GetText().ToString();
		RoomInfo.RoomTitle = FString(RoomTitle);
		RoomInfo.MaxViewers = 10;

		// 현재 PlayerController를 전달하여 이 플레이어만 이동시킴
		APlayerController* PC = GetOwningPlayer();
		SessionManager->CreateSession(RoomInfo);
	}
}
