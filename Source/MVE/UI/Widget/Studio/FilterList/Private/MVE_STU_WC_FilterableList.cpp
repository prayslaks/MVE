
#include "../Public/MVE_STU_WC_FilterableList.h"

#include "MVE.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Widget/Studio/FilterList/Public/MVE_STU_WC_FilterableListItem.h"

void UMVE_STU_WC_FilterableList::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력 필드 이벤트 바인딩
	if (InputField)
	{
		InputField->OnTextCommitted.AddDynamic(this, &UMVE_STU_WC_FilterableList::OnInputCommitted);
		PRINTLOG(TEXT("OnTextCommitted delegate bound!"));
	}
}

void UMVE_STU_WC_FilterableList::AddItem(const FString& ItemData)
{
	// 입력 검증
	if (!ValidateInput(ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid input: %s"), *ItemData);
		return;
	}

	// 최대 개수 체크
	if (ItemWidgets.Num() >= MaxItemCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Maximum item count reached: %d"), MaxItemCount);
		return;
	}

	// 중복 체크
	if (!bAllowDuplicates)
	{
		for (const auto& Widget : ItemWidgets)
		{
			if (Widget && Widget->GetItemData().Equals(ItemData, ESearchCase::IgnoreCase))
			{
				UE_LOG(LogTemp, Warning, TEXT("Duplicate item: %s"), *ItemData);
				return;
			}
		}
	}

	// 항목 위젯 생성
	UMVE_STU_WC_FilterableListItem* NewItemWidget = CreateItemWidget();
	if (!NewItemWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create item widget"));
		return;
	}

	PRINTLOG(TEXT("위젯 생성 함"));

	// 데이터 설정
	PRINTLOG(TEXT("SetItemData 호출 전"));
	NewItemWidget->SetItemData(ItemData);
	PRINTLOG(TEXT("SetItemData 호출 후"));

	NewItemWidget->SetItemIndex(ItemWidgets.Num());
	PRINTLOG(TEXT("SetItemIndex 호출 후"));

	// 델리게이트 바인딩
	NewItemWidget->OnEditRequested.BindUObject(this, &UMVE_STU_WC_FilterableList::HandleItemEdit);
	NewItemWidget->OnDeleteRequested.BindUObject(this, &UMVE_STU_WC_FilterableList::HandleItemDelete);
	NewItemWidget->OnUpdateRequested.BindUObject(this, &UMVE_STU_WC_FilterableList::UpdateItem);
	PRINTLOG(TEXT("델리게이트 바인딩 완료"));

	// ScrollBox에 추가
	if (ItemScrollBox)
	{
		PRINTLOG(TEXT("ScrollBox에 AddChild 시작"));
		ItemScrollBox->AddChild(NewItemWidget);
		PRINTLOG(TEXT("ScrollBox에 AddChild 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ItemScrollBox is nullptr! BindWidget 확인 필요"));
	}

	// 배열에 추가
	ItemWidgets.Add(NewItemWidget);
	PRINTLOG(TEXT("ItemWidgets 배열에 추가 완료"));

	UE_LOG(LogTemp, Log, TEXT("Added item: %s (Total: %d)"), *ItemData, ItemWidgets.Num());
}

void UMVE_STU_WC_FilterableList::RemoveItem(int32 Index)
{
	if (!ItemWidgets.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid item index: %d"), Index);
		return;
	}

	// ScrollBox에서 제거
	if (ItemScrollBox && ItemWidgets[Index])
	{
		ItemScrollBox->RemoveChild(ItemWidgets[Index]);
	}

	// 배열에서 제거
	ItemWidgets.RemoveAt(Index);

	// 나머지 항목들의 인덱스 재설정
	for (int32 i = Index; i < ItemWidgets.Num(); ++i)
	{
		if (ItemWidgets[i])
		{
			ItemWidgets[i]->SetItemIndex(i);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Removed item at index %d (Remaining: %d)"), Index, ItemWidgets.Num());
}

void UMVE_STU_WC_FilterableList::UpdateItem(int32 Index, const FString& NewData)
{
	HandleUpdateItem(Index, NewData);
}

void UMVE_STU_WC_FilterableList::HandleUpdateItem(int32 Index, const FString& NewData)
{
	if (!ItemWidgets.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid item index: %d"), Index);
		return;
	}

	// 입력 검증
	if (!ValidateInput(NewData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid input: %s"), *NewData);
		return;
	}

	// 중복 체크 (자기 자신은 제외)
	if (!bAllowDuplicates)
	{
		for (int32 i = 0; i < ItemWidgets.Num(); ++i)
		{
			if (i != Index && ItemWidgets[i] && ItemWidgets[i]->GetItemData().Equals(NewData, ESearchCase::IgnoreCase))
			{
				UE_LOG(LogTemp, Warning, TEXT("Duplicate item: %s"), *NewData);
				return;
			}
		}
	}

	// 데이터 업데이트
	if (ItemWidgets[Index])
	{
		ItemWidgets[Index]->SetItemData(NewData);
		UE_LOG(LogTemp, Log, TEXT("Updated item at index %d: %s"), Index, *NewData);
	}
}

void UMVE_STU_WC_FilterableList::ClearAllItems()
{
	if (ItemScrollBox)
	{
		ItemScrollBox->ClearChildren();
	}

	ItemWidgets.Empty();

	UE_LOG(LogTemp, Log, TEXT("Cleared all items"));
}

UMVE_STU_WC_FilterableListItem* UMVE_STU_WC_FilterableList::CreateItemWidget()
{
	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemWidgetClass is not set!"));
		return nullptr;
	}

	// 위젯 생성
	UMVE_STU_WC_FilterableListItem* NewWidget = CreateWidget<UMVE_STU_WC_FilterableListItem>(GetOwningPlayer(), ItemWidgetClass);
	return NewWidget;
}

bool UMVE_STU_WC_FilterableList::ValidateInput(const FString& Input) const
{
	// 기본 검증: 빈 문자열 체크
	if (Input.IsEmpty() || Input.TrimStartAndEnd().IsEmpty())
	{
		return false;
	}

	// 자식 클래스에서 추가 검증 가능
	return true;
}

void UMVE_STU_WC_FilterableList::OnInputCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	PRINTLOG(TEXT("OnInputCommitted 함수 호출 됨"));
	// Enter 키로 입력 완료 시에만 처리
	if (CommitType == ETextCommit::OnEnter)
	{
		PRINTLOG(TEXT("OnInputCommitted OnEnter 안에 들어옴"));
		FString InputText = Text.ToString();
		
		// 항목 추가

		AddItem(InputText);

		// 입력 필드 초기화
		if (InputField)
		{
			InputField->SetText(FText::GetEmpty());
		}
	}
}

void UMVE_STU_WC_FilterableList::HandleItemEdit(int32 ItemIndex)
{
	UE_LOG(LogTemp, Log, TEXT("Edit requested for item %d"), ItemIndex);
	
	// 자식 클래스에서 구현
	// 예: 수정 다이얼로그 표시
}

void UMVE_STU_WC_FilterableList::HandleItemDelete(int32 ItemIndex)
{
	UE_LOG(LogTemp, Log, TEXT("Delete requested for item %d"), ItemIndex);
	
	// 항목 삭제
	RemoveItem(ItemIndex);
}


