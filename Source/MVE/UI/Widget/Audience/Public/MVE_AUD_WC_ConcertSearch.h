#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widget/Audience/Public/MVE_AUD_WC_ConcertSearchResult.h"
#include "MVE_AUD_WC_ConcertSearch.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConcertDoubleClicked, const FMVE_AUD_ConcertSearchResultData&, ConcertData);

UCLASS()
class MVE_API UMVE_AUD_WC_ConcertSearch : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnConcertDoubleClicked OnConcertDoubleClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PollingToggleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PollingToggleButtonTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ConcertListScrollBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConcertCountTextBlock;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UMVE_AUD_WC_ConcertSearchResult> ConcertSearchResultWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Polling Button Style")
	FLinearColor ActiveColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Polling Button Style")
	FLinearColor InactiveColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, Category = "Polling Button Style")
	FText ActiveText = FText::FromString(TEXT("폴링 중지"));

	UPROPERTY(EditAnywhere, Category = "Polling Button Style")
	FText InactiveText = FText::FromString(TEXT("폴링 시작"));

	UFUNCTION()
	void OnPollingToggleButtonClicked();
	
	UFUNCTION()
	void OnSearchResultDoubleClicked(UMVE_AUD_WC_ConcertSearchResult* ClickedWidget);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UMVE_AUD_WC_ConcertSearchResult>> SearchResultWidgets;

	UPROPERTY()
	TObjectPtr<UMVE_AUD_WC_ConcertSearchResult> SelectedSearchResult;

	UFUNCTION()
	void GetConcertList();
	
	void OnGetConcertListComplete(const bool bSuccess, const FGetConcertListData& ResponseData, const FString& ErrorCode);
	
	UFUNCTION()
	void UpdatePollingButtonAppearance() const;

	FTimerHandle RefreshTimerHandle;
	bool bIsPollingActive = false;
};