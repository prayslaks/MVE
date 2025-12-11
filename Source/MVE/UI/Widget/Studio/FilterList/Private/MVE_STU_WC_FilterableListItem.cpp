
#include "../Public/MVE_STU_WC_FilterableListItem.h"
#include "Components/Button.h"
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
}

void UMVE_STU_WC_FilterableListItem::RequestEdit()
{
	if (OnEditRequested.IsBound())
	{
		OnEditRequested.Execute(ItemIndex);
	}
}

void UMVE_STU_WC_FilterableListItem::RequestDelete()
{
	if (OnDeleteRequested.IsBound())
	{
		OnDeleteRequested.Execute(ItemIndex);
	}
}