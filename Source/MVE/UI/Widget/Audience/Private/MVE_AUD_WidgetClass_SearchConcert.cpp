
#include "../Public/MVE_AUD_WidgetClass_SearchConcert.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h" // UEditableTextBox를 위해 포함

void UMVE_AUD_WidgetClass_SearchConcert::NativeConstruct()
{
	Super::NativeConstruct();

	if (SearchButton)
	{
		SearchButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_SearchConcert::OnSearchButtonClicked);
	}

    // EditableTextBox의 OnTextCommitted 이벤트에 함수 바인딩
    // 엔터 키를 쳤을 때도 SearchButton 클릭과 동일한 검색 로직을 실행
    if (ConcertSearchEditableTextBox)
    {
        ConcertSearchEditableTextBox->OnTextCommitted.AddDynamic(this, &UMVE_AUD_WidgetClass_SearchConcert::OnEditableTextBoxTextCommitted);
    }

	if (RefreshListButton)
	{
		RefreshListButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_SearchConcert::OnRefreshButtonClicked);
	}
}

// SearchButton 클릭 시 델리게이트를 호출하는 함수
void UMVE_AUD_WidgetClass_SearchConcert::OnSearchButtonClicked()
{
	if (ConcertSearchEditableTextBox)
	{
		// OnSearchCommitted 델리게이트를 호출하여 현재 텍스트를 전달
		OnSearchCommitted.Broadcast(ConcertSearchEditableTextBox->GetText().ToString());
	}
}

// EditableTextBox에서 텍스트가 커밋(엔터 입력 또는 포커스 아웃)되었을 때 호출될 함수
void UMVE_AUD_WidgetClass_SearchConcert::OnEditableTextBoxTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    // 엔터 키 입력으로 커밋된 경우에만 검색 로직 실행
    if (CommitMethod == ETextCommit::OnEnter)
    {
        OnSearchButtonClicked(); // 검색 버튼 클릭과 동일한 로직 실행
    }
}

void UMVE_AUD_WidgetClass_SearchConcert::OnRefreshButtonClicked()
{
	OnRefreshClicked.Broadcast();
}
