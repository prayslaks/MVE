#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WC_ConcertSearchResult.generated.h"

class UBorder;
class UTextBlock;

USTRUCT(BlueprintType)
struct FMVE_AUD_ConcertSearchResultData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ConcertName;

	UPROPERTY(BlueprintReadOnly)
	FString RoomId;

	UPROPERTY(BlueprintReadOnly)
	FString StudioName;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentAudience = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxAudience = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsOpen = false;

	FMVE_AUD_ConcertSearchResultData() = default;

	FMVE_AUD_ConcertSearchResultData(const FString& InConcertName, const FString& InRoomId, const FString& InHostName, int32 InCurrentAudience, int32 InMaxAudience, bool InbIsOpen)
		: ConcertName(InConcertName), RoomId(InRoomId), StudioName(InHostName), CurrentAudience(InCurrentAudience), MaxAudience(InMaxAudience), bIsOpen(InbIsOpen)
	{
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConcertSearchResultDoubleClicked, class UMVE_AUD_WC_ConcertSearchResult*);

UCLASS()
class MVE_API UMVE_AUD_WC_ConcertSearchResult : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnConcertSearchResultDoubleClicked OnConcertSearchResultDoubleClicked;

	void UpdateUI(const FMVE_AUD_ConcertSearchResultData& InConcertSearchResult);
	void SetSelected(bool bInIsSelected);
	const FMVE_AUD_ConcertSearchResultData& GetConcertData() const { return BindingConcertSearchResult; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConcertNameTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HostNameTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AudienceCountTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusTextBlock;
	
	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor DeselectedColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor HoveredColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor SelectedColor = FLinearColor(0.5f, 0.2f, 0.1f, 0.8f);

private:
	bool bIsSelected = false;
	FMVE_AUD_ConcertSearchResultData BindingConcertSearchResult;
};