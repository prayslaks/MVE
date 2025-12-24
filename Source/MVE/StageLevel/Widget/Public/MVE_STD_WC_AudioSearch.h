// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioSearch.generated.h"

class UMVE_STU_WC_ConcertStudioPanel;
class UButton;
class UScrollBox;
class UMVE_STD_WC_AudioSearchResult;
struct FMVE_STD_AudioSearchResultData; // Forward declaration for delegate

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioSearchResultSelected, const FMVE_STD_AudioSearchResultData&, AudioData);

UCLASS()
class MVE_API UMVE_STD_WC_AudioSearch : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable, Category = "Audio Search")
	FOnAudioSearchResultSelected OnAudioSearchResultSelected;

	/** 검색 결과 위젯 배열 (STT 자동 선택을 위해 public) */
	UPROPERTY()
	TArray<TObjectPtr<UMVE_STD_WC_AudioSearchResult>> SearchResultWidgets;

	/** PlaylistBuilder에서 전달받은 재생목록 설정 */
	UFUNCTION(BlueprintCallable, Category = "Audio Search")
	void SetPlaylistFromBuilder(const TArray<FAudioFile>& Playlist);

protected:
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> GetAudioListButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> AudioListScrollBox;
	
	UPROPERTY(EditAnywhere, Category="AudioSearch")
	TSubclassOf<UMVE_STD_WC_AudioSearchResult> AudioSearchResultWidgetClass;
	
	UFUNCTION()
	void OnGetAudioListButtonClicked();

private:
	UPROPERTY()
	TObjectPtr<UMVE_STD_WC_AudioSearchResult> SelectedSearchResult;

	UFUNCTION()
	void OnSearchResultDoubleClicked(UMVE_STD_WC_AudioSearchResult* ClickedWidget);
};