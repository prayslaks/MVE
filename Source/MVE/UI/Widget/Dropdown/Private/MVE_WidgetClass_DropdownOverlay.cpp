
#include "../Public/MVE_WidgetClass_DropdownOverlay.h"
#include "Components/Button.h"

void UMVE_WidgetClass_DropdownOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Overlay)
	{
		Btn_Overlay->OnClicked.AddDynamic(this, &UMVE_WidgetClass_DropdownOverlay::OnOverlayClicked);
	}
}


void UMVE_WidgetClass_DropdownOverlay::OnOverlayClicked()
{
	OnOverlayClickedEvent.Broadcast();
}

void UMVE_WidgetClass_DropdownOverlay::Transition_Implementation(EUIScreen TargetUIScreen)
{
}
