
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_StationLevel_AudienceStation.generated.h"


class UMVE_AUD_WidgetClass_ConcertList;
class UMVE_AUD_WidgetClass_SearchConcert;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_StationLevel_AudienceStation : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMVE_AUD_WidgetClass_SearchConcert> SearchConcertWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMVE_AUD_WidgetClass_ConcertList> ConcertListWidget;
};
