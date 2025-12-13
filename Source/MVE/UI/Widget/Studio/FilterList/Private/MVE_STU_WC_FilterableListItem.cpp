
#include "../Public/MVE_STU_WC_FilterableListItem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"


void UMVE_STU_WC_FilterableListItem::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (InteractionButton)
	{
		InteractionButton->OnHovered.AddDynamic(this, &UMVE_STU_WC_FilterableListItem::OnItemHovered);
		InteractionButton->OnUnhovered.AddDynamic(this, &UMVE_STU_WC_FilterableListItem::OnItemUnhovered);
		InteractionButton->OnClicked.AddDynamic(this, &UMVE_STU_WC_FilterableListItem::OnItemClicked);
	}

	// EditField 이벤트 바인딩
	if (EditField)
	{
		EditField->OnTextCommitted.AddDynamic(this, &UMVE_STU_WC_FilterableListItem::OnEditFieldCommitted);
		
		// 초기에는 숨김
		EditField->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMVE_STU_WC_FilterableListItem::SetItemData(const FString& Data)
{
	ItemData = Data;

	// 기본 구현: DisplayText에 데이터 표시
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(Data));
	}
}

void UMVE_STU_WC_FilterableListItem::SetItemIndex(int32 Index)
{
	ItemIndex = Index;
}

void UMVE_STU_WC_FilterableListItem::OnItemHovered()
{
	// 자식 클래스에서 호버 효과 추가 구현 가능
	// 예: 배경색 변경, 애니메이션 재생 등
}

void UMVE_STU_WC_FilterableListItem::OnItemUnhovered()
{
	// 자식 클래스에서 호버 해제 효과 추가 구현 가능
}

void UMVE_STU_WC_FilterableListItem::OnItemClicked()
{
	// UIManagerSubsystem의 드롭다운 시스템 호출
	// 드롭다운 메뉴: 수정, 삭제
	// 실제 드롭다운 연동은 자식 클래스 또는 Blueprint에서 구현
	
	UE_LOG(LogTemp, Log, TEXT("Item %d clicked"), ItemIndex);
	HandleItemClicked();
}

void UMVE_STU_WC_FilterableListItem::HandleItemClicked()
{
	// 자식 클래스에서 구현.
}

void UMVE_STU_WC_FilterableListItem::RequestEdit()
{
	if (OnEditRequested.IsBound())
	{
		OnEditRequested.Execute(ItemIndex);
	}

	SwitchToEditMode();
}

void UMVE_STU_WC_FilterableListItem::RequestDelete()
{
	if (OnDeleteRequested.IsBound())
	{
		OnDeleteRequested.Execute(ItemIndex);
	}
}

void UMVE_STU_WC_FilterableListItem::SwitchToEditMode()
{
	if (!EditField)
	{
		UE_LOG(LogTemp, Warning, TEXT("EditField is not available for this item"));
		return;
	}

	bIsEditMode = true;

	// EditField에 현재 텍스트 설정
	EditField->SetText(FText::FromString(ItemData));

	// Visibility 전환
	if (DisplayText)
	{
		DisplayText->SetVisibility(ESlateVisibility::Collapsed);
	}

	EditField->SetVisibility(ESlateVisibility::Visible);

	// EditField에 포커스 이동
	EditField->SetKeyboardFocus();

	UE_LOG(LogTemp, Log, TEXT("Item %d switched to Edit Mode"), ItemIndex);
}

void UMVE_STU_WC_FilterableListItem::SwitchToDisplayMode()
{
	bIsEditMode = false;

	// Visibility 전환
	if (DisplayText)
	{
		DisplayText->SetVisibility(ESlateVisibility::Visible);
	}

	if (EditField)
	{
		EditField->SetVisibility(ESlateVisibility::Collapsed);
	}

	UE_LOG(LogTemp, Log, TEXT("Item %d switched to Display Mode"), ItemIndex);

}

ESlateVisibility UMVE_STU_WC_FilterableListItem::GetDisplayTextVisibility() const
{
	return bIsEditMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
}

ESlateVisibility UMVE_STU_WC_FilterableListItem::GetEditFieldVisibility() const
{
	return bIsEditMode ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

void UMVE_STU_WC_FilterableListItem::OnEditFieldCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	FString NewText = Text.ToString().TrimStartAndEnd();

	// Enter 키로 확정
	if (CommitType == ETextCommit::OnEnter)
	{
		// 빈 텍스트 방지
		if (NewText.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Empty text not allowed, reverting to original"));
			SwitchToDisplayMode();
			return;
		}

		// 변경사항이 있는 경우에만 업데이트 요청
		if (!NewText.Equals(ItemData))
		{
			UE_LOG(LogTemp, Log, TEXT("Updating item %d: %s -> %s"), ItemIndex, *ItemData, *NewText);
			
			// 부모 리스트에게 업데이트 요청
			// 이를 위해 델리게이트 추가 필요
			OnUpdateRequested.ExecuteIfBound(ItemIndex, NewText);
		}

		SwitchToDisplayMode();
	}
	// ESC 키나 포커스 잃을 때는 취소
	else if (CommitType == ETextCommit::OnUserMovedFocus)
	{
		UE_LOG(LogTemp, Log, TEXT("Edit cancelled (focus lost)"));
		SwitchToDisplayMode();
	}
}
