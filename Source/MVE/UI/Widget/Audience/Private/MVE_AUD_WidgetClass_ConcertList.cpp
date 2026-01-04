
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

	// 광고 데이터 로드
	LoadAdvertisements();

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

void UMVE_AUD_WidgetClass_ConcertList::UpdateConcertList(const TArray<FConcertInfo>& Concerts, bool bShowAd)
{
	if (!RoomTileView)
	{
		PRINTLOG(TEXT("RoomTileView is nullptr!"));
		return;
	}

	// 기존 데이터 초기화
	ClearConcertList();

	if (bShowAd)
	{
		// 1. 광고 먼저 추가 (캐시된 광고 데이터 사용)
		for (UConcertInfoData* AdData : CachedAdvertisements)
		{
			if (AdData)
			{
				RoomTileView->AddItem(AdData);
			}
		}
	}
	

	// 2. 실제 콘서트 목록 추가
	for (const FConcertInfo& ConcertInfo : Concerts)
	{
		// UObject 래퍼 생성
		UConcertInfoData* ConcertData = NewObject<UConcertInfoData>(this);

		// 구조체 데이터 설정
		ConcertData->SetConcertInfo(ConcertInfo);

		// TileView에 추가
		RoomTileView->AddItem(ConcertData);
	}

	PRINTLOG(TEXT("Concert list updated: %d advertisements + %d concerts"), CachedAdvertisements.Num(), Concerts.Num());
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
		TArray<FConcertInfo> OnAirConcertList;
		for (auto Info : ConcertList)
		{
			if (Info.IsOpen)
			{
				OnAirConcertList.Add(Info);
			}
		}
		UpdateConcertList(OnAirConcertList, true);
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

void UMVE_AUD_WidgetClass_ConcertList::LoadAdvertisements()
{
	// 기존 광고 데이터 초기화
	CachedAdvertisements.Empty();

	if (!AdvertisementDataTable)
	{
		PRINTLOG(TEXT("AdvertisementDataTable is not set. Skipping advertisement loading."));
		return;
	}

	// DataTable의 모든 Row를 순회
	TArray<FName> RowNames = AdvertisementDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		// DataTable에서 광고 데이터 가져오기
		FDTAdvertisementConcert* AdRow = AdvertisementDataTable->FindRow<FDTAdvertisementConcert>(RowName, TEXT("LoadAdvertisements"));

		if (!AdRow)
		{
			PRINTLOG(TEXT("Failed to load advertisement row: %s"), *RowName.ToString());
			continue;
		}

		// FConcertInfo로 변환
		FConcertInfo AdConcertInfo;
		AdConcertInfo.RoomId = FString::Printf(TEXT("AD_%s"), *RowName.ToString()); // 광고 식별용 RoomId
		AdConcertInfo.ConcertName = AdRow->AdTitle;
		AdConcertInfo.StudioName = AdRow->AdvertiserName;
		AdConcertInfo.IsOpen = AdRow->bIsClickable; // 클릭 가능 여부
		AdConcertInfo.CurrentAudience = 0;
		AdConcertInfo.StudioUserId = -1; // 광고는 UserId -1로 표시
		AdConcertInfo.IsAdvertisement = true;
		AdConcertInfo.AdvertisementImage = AdRow->ThumbnailTexture;
		AdConcertInfo.AdvertisementText = AdRow->AdvertisementText;

		// UConcertInfoData로 래핑
		UConcertInfoData* AdData = NewObject<UConcertInfoData>(this);
		AdData->SetConcertInfo(AdConcertInfo);

		// 캐시에 저장
		CachedAdvertisements.Add(AdData);

		// TileView에 추가
		if (RoomTileView)
		{
			RoomTileView->AddItem(AdData);
		}
	}

	PRINTLOG(TEXT("Loaded %d advertisements"), CachedAdvertisements.Num());
}
