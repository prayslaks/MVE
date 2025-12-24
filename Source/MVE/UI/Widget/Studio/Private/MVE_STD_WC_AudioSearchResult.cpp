// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/Studio/Public/MVE_STD_WC_AudioSearchResult.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "MVE_PlaylistDragDropOperation.h"
#include "MVE_STD_WC_PlaylistBuilder.h"
#include "MVE.h"

void UMVE_STD_WC_AudioSearchResult::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 삭제 버튼 이벤트 바인딩 및 숨기기
	if (DeleteButton)
	{
		DeleteButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_AudioSearchResult::OnDeleteButtonClicked);
		DeleteButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMVE_STD_WC_AudioSearchResult::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(HoveredColor);
	}

	// 재생목록에서만 삭제 버튼 표시
	if (bIsDraggable && DeleteButton)
	{
		DeleteButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMVE_STD_WC_AudioSearchResult::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (!bIsSelected && BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(DeselectedColor);
	}

	// 삭제 버튼 숨기기
	if (DeleteButton)
	{
		DeleteButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMVE_STD_WC_AudioSearchResult::UpdateUI(const FMVE_STD_AudioSearchResultData& InAudioSearchResult)
{
	// 업데이트
	BindingAudioSearchResult = InAudioSearchResult;
	
	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(FText::FromString(BindingAudioSearchResult.Title));
	}
	
	if (ArtistTextBlock)
	{
		ArtistTextBlock->SetText(FText::FromString(BindingAudioSearchResult.Artist));
	}
	
	if (DurationTextBlock)
	{
		// Duration is in seconds, format as MM:SS
		const int32 TotalSeconds = BindingAudioSearchResult.Duration;
		const int32 Minutes = TotalSeconds / 60;
		const int32 Seconds = TotalSeconds % 60;
		const FString FormattedDuration = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DurationTextBlock->SetText(FText::FromString(FormattedDuration));
	}
	
	SetSelected(false);
}

void UMVE_STD_WC_AudioSearchResult::SetSelected(const bool bInIsSelected)
{
	bIsSelected = bInIsSelected;
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
	}
}

FReply UMVE_STD_WC_AudioSearchResult::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnAudioSearchResultClicked.Broadcast(this);

		// 드래그 가능한 경우 드래그 감지 시작
		if (bIsDraggable)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(MouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}

		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply UMVE_STD_WC_AudioSearchResult::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnAudioSearchResultDoubleClicked.Broadcast(this);
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UMVE_STD_WC_AudioSearchResult::NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, UDragDropOperation*& Operation)
{
	Super::NativeOnDragDetected(MyGeometry, MouseEvent, Operation);

	if (!bIsDraggable)
	{
		return;
	}

	// DragDropOperation 생성
	UMVE_PlaylistDragDropOperation* DragOp = NewObject<UMVE_PlaylistDragDropOperation>();
	if (DragOp)
	{
		DragOp->DraggedAudioData = FAudioFile();
		DragOp->DraggedAudioData.Id = BindingAudioSearchResult.Id;
		DragOp->DraggedAudioData.Title = BindingAudioSearchResult.Title;
		DragOp->DraggedAudioData.Artist = BindingAudioSearchResult.Artist;
		DragOp->DraggedAudioData.Duration = BindingAudioSearchResult.Duration;
		DragOp->OriginalIndex = PlaylistIndex;
		DragOp->DraggedWidget = this;

		// 드래그 비주얼 설정 (현재 위젯의 복사본, 반투명)
		DragOp->DefaultDragVisual = this;
		DragOp->Pivot = EDragPivot::MouseDown;
		DragOp->Offset = FVector2D::ZeroVector;

		Operation = DragOp;

		PRINTLOG(TEXT("드래그 시작: %s (Index: %d)"), *BindingAudioSearchResult.Title, PlaylistIndex);
	}
}

bool UMVE_STD_WC_AudioSearchResult::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UMVE_PlaylistDragDropOperation* DragOp = Cast<UMVE_PlaylistDragDropOperation>(InOperation);
	if (!DragOp || !bIsDraggable)
	{
		return false;
	}

	// 자기 자신에게 드롭하면 무시
	if (DragOp->OriginalIndex == PlaylistIndex)
	{
		if (BackgroundBorder)
		{
			BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
		}
		return false;
	}

	PRINTLOG(TEXT("드롭됨: From %d → To %d"), DragOp->OriginalIndex, PlaylistIndex);

	// 배경색 복원
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
	}

	// PlaylistBuilder에 순서 변경 요청
	if (OwningPlaylistBuilder)
	{
		OwningPlaylistBuilder->ReorderPlaylistItem(DragOp->OriginalIndex, PlaylistIndex);
		PRINTLOG(TEXT("순서 변경 완료: %d → %d"), DragOp->OriginalIndex, PlaylistIndex);
		return true;
	}
	else
	{
		PRINTLOG(TEXT("OwningPlaylistBuilder가 null입니다!"));
		return false;
	}
}

void UMVE_STD_WC_AudioSearchResult::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	UMVE_PlaylistDragDropOperation* DragOp = Cast<UMVE_PlaylistDragDropOperation>(InOperation);
	if (!DragOp || !bIsDraggable)
	{
		return;
	}

	// 드롭 가능 하이라이트
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(DropTargetColor);
	}
}

void UMVE_STD_WC_AudioSearchResult::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	// 하이라이트 제거
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsSelected ? SelectedColor : DeselectedColor);
	}
}

void UMVE_STD_WC_AudioSearchResult::OnDeleteButtonClicked()
{
	PRINTLOG(TEXT("삭제 버튼 클릭됨: %s (Index: %d)"), *BindingAudioSearchResult.Title, PlaylistIndex);
	OnDeleteRequested.Broadcast(this);
}