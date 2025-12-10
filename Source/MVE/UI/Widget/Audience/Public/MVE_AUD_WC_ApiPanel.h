#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearchResult.h" // For the struct
#include "MVE_AUD_WC_ApiPanel.generated.h"

class UMVE_AUD_WC_ConcertSearch;

UCLASS()
class MVE_API UMVE_AUD_WC_ApiPanel : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMVE_AUD_WC_ConcertSearch> ConcertSearch;

	UFUNCTION()
	void HandleConcertDoubleClicked(const FMVE_AUD_ConcertSearchResultData& ConcertData);
};