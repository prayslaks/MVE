
#include "../Public/MVE_AUD_WidgetClass_ConcertList.h"
#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "MVE_GIS_SessionManager.h"
#include "Components/TileView.h"

void UMVE_AUD_WidgetClass_ConcertList::NativeConstruct()
{
	Super::NativeConstruct();

	if (RoomTileView)
	{
		// 이벤트 바인딩
		RoomTileView->OnItemClicked().AddUObject(this, &UMVE_AUD_WidgetClass_ConcertList::OnConcertItemClicked);
		PRINTLOG(TEXT("TileView initialized."));
	}

	// SessionManager 델리게이트 바인딩 및 세션 찾기
	if (UGameInstance* GI = GetGameInstance())
	{
		SessionManager = GI->GetSubsystem<UMVE_GIS_SessionManager>();
		if (SessionManager)
		{
			//델리게이트 바인딩
			SessionManager->OnSessionsFound.AddDynamic(this, &UMVE_AUD_WidgetClass_ConcertList::OnSessionsFoundCallback);

			// 세션 목록 조회 시작
			RefreshSessionList();

			PRINTLOG(TEXT("SessionManager delegate bound and FindSessions called."));
		}
		else
		{
			PRINTLOG(TEXT("Failed to get SessionManager subsystem!"));
		}
	}
}

void UMVE_AUD_WidgetClass_ConcertList::NativeDestruct()
{
	// 델리게이트 언바인딩
	if (SessionManager)
	{
		SessionManager->OnSessionsFound.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMVE_AUD_WidgetClass_ConcertList::UpdateConcertList(const TArray<FConcertInfo>& Concerts)
{
	if (!RoomTileView)
	{
		PRINTLOG(TEXT("RoomTileView is nullptr!"));
		return;
	}

	// 기존 데이터 초기화
	ClearConcertList();

	// FConcertInfo → UConcertInfoData 변환하여 TileView에 추가
	for (const FConcertInfo& ConcertInfo : Concerts)
	{
		// UObject 래퍼 생성
		UConcertInfoData* ConcertData = NewObject<UConcertInfoData>(this);
        
		// 구조체 데이터 설정
		ConcertData->SetConcertInfo(ConcertInfo);

		// TileView에 추가
		RoomTileView->AddItem(ConcertData);
	}

	PRINTLOG(TEXT("Concert list updated: %d concerts"), Concerts.Num());
}

void UMVE_AUD_WidgetClass_ConcertList::ClearConcertList()
{
	if (RoomTileView)
	{
		RoomTileView->ClearListItems();
	}
}

void UMVE_AUD_WidgetClass_ConcertList::RefreshSessionList()
{
	if (SessionManager)
	{
		PRINTLOG(TEXT("Refreshing session list..."));
		SessionManager->FindSessions();
	}
}

void UMVE_AUD_WidgetClass_ConcertList::OnSessionsFoundCallback(bool bSuccess, int32 NumSessions)
{
	if (!SessionManager)
	{
		PRINTLOG(TEXT("SessionManager is nullptr in callback!"));
		return;
	}

	if (bSuccess)
	{
		PRINTLOG(TEXT("Sessions found: %d"), NumSessions);
        
		// SessionManager에서 콘서트 목록 가져오기
		const TArray<FConcertInfo>& ConcertList = SessionManager->GetConcertList();
		UpdateConcertList(ConcertList);
	}
	else
	{
		PRINTLOG(TEXT("Failed to find sessions!"));
		ClearConcertList();
	}
}

void UMVE_AUD_WidgetClass_ConcertList::OnConcertItemClicked(UObject* ClickedItem)
{
	UConcertInfoData* ConcertData = Cast<UConcertInfoData>(ClickedItem);
	if (!ConcertData)
	{
		PRINTLOG(TEXT("ClickedItem is not UConcertInfoData!"));
		return;
	}

	const FConcertInfo& ConcertInfo = ConcertData->GetConcertInfo();
	PRINTLOG(TEXT("Concert clicked: %s (RoomId: %s)"),
		   *ConcertInfo.ConcertName, *ConcertInfo.RoomId);

	// 선택 이벤트 브로드캐스트
	//OnConcertSelected.Broadcast(ConcertData);
}
