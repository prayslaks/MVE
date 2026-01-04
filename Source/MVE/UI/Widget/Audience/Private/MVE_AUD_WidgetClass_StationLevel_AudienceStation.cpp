#include "../Public/MVE_AUD_WidgetClass_StationLevel_AudienceStation.h"

#include "Components/Button.h"
#include "../Public/MVE_AUD_WidgetClass_ConcertList.h" // ConcertListWidget 사용을 위해 추가
#include "../Public/MVE_AUD_WidgetClass_SearchConcert.h" // SearchConcertWidget 사용을 위해 추가
#include "MVE_API_Helper.h" // API 헬퍼 사용을 위해 추가
#include "UIManagerSubsystem.h"
#include "Data/ScreenTypes.h"

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::NativeConstruct()
{
	Super::NativeConstruct();

	if (CustomButton)
	{
		CustomButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnCustomButtonClicked);
	}

	if (ThrowCustomButton)
	{
		ThrowCustomButton->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnThrowCustomButtonClicked);
	}

    // SearchConcertWidget의 델리게이트에 핸들러 함수 바인딩
    if (SearchConcertWidget)
    {
        SearchConcertWidget->OnSearchCommitted.AddDynamic(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::HandleSearchCommitted);
    }

    // 위젯이 생성될 때, 전체 콘서트 목록을 서버에서 가져오기
    // FOnGetConcertListComplete 델리게이트를 생성하여 콜백 함수 지정
    UMVE_API_Helper::GetConcertList(FOnGetConcertListComplete::CreateUObject(this, &UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnGetConcertListComplete));
}

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnCustomButtonClicked()
{
    if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
    {
        UIManager->ShowScreen(EUIScreen::AudienceCustomizing);
    }
}

void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnThrowCustomButtonClicked()
{
    if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
    {
        UIManager->ShowScreen(EUIScreen::AudienceThrowMeshGenScreen);
    }
}

// 검색어 입력 처리 함수 구현
void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::HandleSearchCommitted(const FString& SearchText)
{
    if (!ConcertListWidget)
    {
        return;
    }

    // 검색어가 비어있으면 전체 목록 표시
    if (SearchText.IsEmpty())
    {
        ConcertListWidget->UpdateConcertList(AllConcertList, true);
        return;
    }

    // 필터링된 목록을 저장할 새 배열
    TArray<FConcertInfo> FilteredList;

    // 전체 목록(AllConcertList)에서 검색어(SearchText)를 포함하는 항목만 골라내기
    for (const FConcertInfo& Concert : AllConcertList)
    {
        // 대소문자 구분 없이 검색하려면 ToLower()를 사용
        if (Concert.ConcertName.Contains(SearchText, ESearchCase::IgnoreCase))
        {
            FilteredList.Add(Concert);
        }
    }

    // 필터링된 결과로 ConcertList 위젯 업데이트
    ConcertListWidget->UpdateConcertList(FilteredList, false);
}

// API 호출 완료 시 실행될 콜백 함수 구현
void UMVE_AUD_WidgetClass_StationLevel_AudienceStation::OnGetConcertListComplete(bool bSuccess, const FGetConcertListData& ResponseData, const FString& ErrorCode)
{
    if (bSuccess && ConcertListWidget)
    {
        // 1. 가져온 전체 목록을 AllConcertList에 저장
        AllConcertList = ResponseData.Concerts;

        // 2. 초기 화면에는 전체 목록을 표시
        ConcertListWidget->UpdateConcertList(AllConcertList, true);
    }
    // TODO: else 블록에서 API 호출 실패 처리 (예: 에러 메시지 표시)
}
