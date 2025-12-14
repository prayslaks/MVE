
#include "../Public/MVE_STD_WidgetClass_FinalCheckSettings.h"

#include "MVE_API_Helper.h"
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
	FConcertInfo ConcertInfo;
	ConcertInfo.ConcertName = RoomTitleEditableText->GetText().ToString();
	ConcertInfo.MaxAudience = 20;
	ConcertInfo.Songs = TArray<FConcertSong>();
	ConcertInfo.Accessories = TArray<FAccessory>();

	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		SessionManager->CreateSession(ConcertInfo);
	}
}
