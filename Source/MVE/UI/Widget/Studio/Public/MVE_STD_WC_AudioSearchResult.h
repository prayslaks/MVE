// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioSearchResult.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UMVE_STD_WC_AudioSearchResult;

// 위젯 더블 클릭 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAudioSearchResultDoubleClicked, UMVE_STD_WC_AudioSearchResult*);

// 위젯 클릭 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAudioSearchResultClicked, UMVE_STD_WC_AudioSearchResult*);

USTRUCT(BlueprintType)
struct FMVE_STD_AudioSearchResultData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString Title;
	
	UPROPERTY(BlueprintReadOnly)
	FString Artist;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Id;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Duration;
	
	// 기본 생성자
	FMVE_STD_AudioSearchResultData() : Id(0), Duration(0) { }
	
	// 초기화 생성자
	FMVE_STD_AudioSearchResultData(const FString& InTitle, const FString& InArtist, const int32& InId, const int32& InDuration) : 
		Title(InTitle), Artist(InArtist), Id(InId), Duration(InDuration) { }
};

UCLASS()
class MVE_API UMVE_STD_WC_AudioSearchResult : public UUserWidget
{
	GENERATED_BODY()

public:
	// 더블 클릭 델리게이트
	FOnAudioSearchResultDoubleClicked OnAudioSearchResultDoubleClicked;

	// 클릭 델리게이트
	FOnAudioSearchResultClicked OnAudioSearchResultClicked;

	UFUNCTION()
	void UpdateUI(const FMVE_STD_AudioSearchResultData& InAudioSearchResult);

	void SetSelected(bool bInIsSelected);

	FMVE_STD_AudioSearchResultData GetAudioData() const { return BindingAudioSearchResult; }

protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ArtistTextBlock;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor SelectedColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor DeselectedColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor HoveredColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f);
	
private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	UPROPERTY()
	FMVE_STD_AudioSearchResultData BindingAudioSearchResult;

	bool bIsSelected = false;
};
