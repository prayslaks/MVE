
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MVE_AUD_WidgetController_Station.generated.h"


class UMVE_AUD_WidgetClass_SearchConcert;
class UMVE_AUD_WidgetClass_ConcertList;

UCLASS(Blueprintable, BlueprintType)
class MVE_API UMVE_AUD_WidgetController_Station : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Init(UMVE_AUD_WidgetClass_ConcertList* InConcertListWidget,
		UMVE_AUD_WidgetClass_SearchConcert* InSearchConcertWidget);

	UPROPERTY()
	TObjectPtr<UMVE_AUD_WidgetClass_ConcertList> ConcertListWidget;

	UPROPERTY()
	TObjectPtr<UMVE_AUD_WidgetClass_SearchConcert> SearchConcertWidget;

private:
	
	UFUNCTION()
	void HandleRefreshClicked();
};
