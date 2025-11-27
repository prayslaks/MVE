
#include "../Public/MVE_AUD_WidgetClass_SearchConcert.h"

#include "Components/Button.h"

void UMVE_AUD_WidgetClass_SearchConcert::NativeConstruct()
{
	Super::NativeConstruct();

	if (RefreshListButton)
		RefreshListButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_SearchConcert::OnRefreshButtonClicked);
	
}

void UMVE_AUD_WidgetClass_SearchConcert::OnRefreshButtonClicked()
{
	OnRefreshClicked.Broadcast();
}
