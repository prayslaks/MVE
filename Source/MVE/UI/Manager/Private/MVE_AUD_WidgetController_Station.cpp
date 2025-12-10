
#include "../Public/MVE_AUD_WidgetController_Station.h"

#include "MVE.h"
#include "MVE_AUD_WidgetClass_ConcertList.h"
#include "MVE_AUD_WidgetClass_SearchConcert.h"

void UMVE_AUD_WidgetController_Station::Init(UMVE_AUD_WidgetClass_ConcertList* InConcertListWidget,
                                             UMVE_AUD_WidgetClass_SearchConcert* InSearchConcertWidget)
{
	ConcertListWidget = InConcertListWidget;
	SearchConcertWidget = InSearchConcertWidget;

	if (SearchConcertWidget)
		SearchConcertWidget->OnRefreshClicked.AddDynamic(this, &UMVE_AUD_WidgetController_Station::HandleRefreshClicked);

}

void UMVE_AUD_WidgetController_Station::HandleRefreshClicked()
{
	if (ConcertListWidget)
	{
		ConcertListWidget->RefreshSessionList();
	}
	
	PRINTLOG(TEXT("HandleRefreshClicked Called"));
}
