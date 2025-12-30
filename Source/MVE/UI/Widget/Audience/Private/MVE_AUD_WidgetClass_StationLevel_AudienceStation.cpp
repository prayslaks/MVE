
#include "../Public/MVE_AUD_WidgetClass_StationLevel_AudienceStation.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::NativeConstruct()
{
	Super::NativeConstruct();

	if (CustomButton)
		CustomButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnCustomButtonClicked);

	if (ThrowCustomButton)
		ThrowCustomButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnThrowCustomButtonClicked);
}

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnCustomButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceCustomizing);
	}
}

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnThrowCustomButtonClicked()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceThrowMeshGenScreen);
	}
}
