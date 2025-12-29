// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioSearchResult.generated.h"

class UButton;
class UBorder;
class UImage;
class UTextBlock;
class UMVE_STD_WC_AudioSearchResult;

// 위젯 더블 클릭 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAudioSearchResultDoubleClicked, UMVE_STD_WC_AudioSearchResult*);

// 위젯 클릭 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAudioSearchResultClicked, UMVE_STD_WC_AudioSearchResult*);

// 삭제 요청 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeleteRequested, UMVE_STD_WC_AudioSearchResult*);

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

	// 삭제 요청 델리게이트
	FOnDeleteRequested OnDeleteRequested;

	UFUNCTION()
	void UpdateUI(const FMVE_STD_AudioSearchResultData& InAudioSearchResult);

	void SetSelected(bool bInIsSelected);

	FMVE_STD_AudioSearchResultData GetAudioData() const { return BindingAudioSearchResult; }

	// 드래그 가능 여부 설정 (재생목록에서만 true)
	void SetDraggable(bool bInIsDraggable) { bIsDraggable = bInIsDraggable; }

	// 재생목록에서의 인덱스 설정
	void SetPlaylistIndex(int32 InIndex) { PlaylistIndex = InIndex; }

	// 재생목록 인덱스 가져오기
	int32 GetPlaylistIndex() const { return PlaylistIndex; }

	// PlaylistBuilder 포인터 설정
	void SetPlaylistBuilder(class UMVE_STD_WC_PlaylistBuilder* InPlaylistBuilder) { OwningPlaylistBuilder = InPlaylistBuilder; }

protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ArtistTextBlock;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DurationTextBlock;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> DeleteButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor SelectedColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor DeselectedColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor HoveredColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor DropTargetColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.3f);

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 삭제 버튼 클릭 이벤트
	UFUNCTION()
	void OnDeleteButtonClicked();

	// 드래그 앤 드롭 관련
	virtual void NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, UDragDropOperation*& Operation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY()
	FMVE_STD_AudioSearchResultData BindingAudioSearchResult;

	// PlaylistBuilder 참조 (드래그 앤 드롭용)
	UPROPERTY()
	TObjectPtr<class UMVE_STD_WC_PlaylistBuilder> OwningPlaylistBuilder;

	bool bIsSelected = false;
	bool bIsDraggable = false;
	int32 PlaylistIndex = -1;
};
