
#include "../Public/MVE_AUD_WidgetClass_ConcertRoom.h"

#include "MVE_AUD_AudienceCharacter.h"
#include "Character/Public/MVE_AUD_InteractionComponent.h"
#include "Components/Button.h"

void UMVE_AUD_WidgetClass_ConcertRoom::NativeConstruct()
{
	Super::NativeConstruct();

	if (WaveButton)
	{
		WaveButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_ConcertRoom::OnWaveButtonClicked);
	}
}

void UMVE_AUD_WidgetClass_ConcertRoom::OnWaveButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	
	AMVE_AUD_AudienceCharacter* AudienceChar  = Cast<AMVE_AUD_AudienceCharacter>(PC->GetPawn());
	if (AudienceChar && AudienceChar->GetInteractionComponent())
	{
		AudienceChar->GetInteractionComponent()->RequestWaveLightStick();
	}
}
