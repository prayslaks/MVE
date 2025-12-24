// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_API_ResponseData.h"
#include "MVE_STD_WC_PlaylistBuilder.generated.h"

class UEditableTextBox;
class UScrollBox;
class UVerticalBox;
class UMVE_STD_WC_AudioSearchResult;

UCLASS()
class MVE_API UMVE_STD_WC_PlaylistBuilder : public UUserWidget
{
	GENERATED_BODY()

public:
	// 재생목록 가져오기
	UFUNCTION(BlueprintCallable, Category = "Playlist")
	TArray<FAudioFile> GetPlaylist() const { return Playlist; }

	// 재생목록 초기화
	UFUNCTION(BlueprintCallable, Category = "Playlist")
	void ClearPlaylist();

	// SessionManager에 재생목록 저장 (StageLevel 이동 전 호출)
	UFUNCTION(BlueprintCallable, Category = "Playlist")
	void SavePlaylistToSessionManager();

	// 재생목록 항목 순서 재배열
	void ReorderPlaylistItem(int32 FromIndex, int32 ToIndex);

	// 재생목록 항목 삭제
	void RemovePlaylistItem(int32 Index);

protected:
	virtual void NativeConstruct() override;

	// 위젯 바인딩
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> SearchBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> DropdownPanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> PlaylistScrollBox;

	// 검색 결과 항목 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playlist")
	TSubclassOf<UMVE_STD_WC_AudioSearchResult> SearchResultWidgetClass;

	// 재생목록 항목 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playlist")
	TSubclassOf<UMVE_STD_WC_AudioSearchResult> PlaylistItemWidgetClass;

private:
	// 서버에서 음악 리스트 로드
	void LoadMusicListFromServer();

	// 음악 리스트 로드 완료 콜백
	void OnMusicListLoaded(bool bSuccess, const FAudioListResponseData& ResponseData, const FString& ErrorCode);

	// 검색 텍스트 변경 이벤트
	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	// 검색 결과 항목 클릭 이벤트 (싱글클릭)
	void OnSearchResultClicked(UMVE_STD_WC_AudioSearchResult* ClickedWidget);

	// 재생목록 항목 드롭 이벤트
	void OnPlaylistItemDropped(int32 FromIndex, int32 ToIndex);

	// 재생목록 항목 삭제 요청 이벤트
	void OnPlaylistItemDeleteRequested(UMVE_STD_WC_AudioSearchResult* Widget);

	// 드롭다운 업데이트
	void UpdateDropdown(const TArray<FAudioFile>& Results);

	// 재생목록 ScrollBox 갱신 (인덱스 재설정)
	void RefreshPlaylistIndices();

	// 재생목록에 음악 추가
	void AddToPlaylist(const FAudioFile& AudioFile);

	// 캐시된 전체 음악 리스트
	UPROPERTY()
	TArray<FAudioFile> CachedMusicList;

	// 현재 재생목록
	UPROPERTY()
	TArray<FAudioFile> Playlist;
};
