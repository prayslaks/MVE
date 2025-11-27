
#include "../Public/MVE_AUD_AudienceStationModel.h"

#include "MVE.h"
#include "MVE_GIS_SessionManager.h"


void UMVE_AUD_AudienceStationModel::Initialize(UMVE_GIS_SessionManager* InSessionManager)
{
	SessionManager = InSessionManager;

	if (!SessionManager)
	{
		return;
	}

	BindSessionManagerEvents();
}

void UMVE_AUD_AudienceStationModel::RefreshRoomList()
{
	if (!SessionManager) return;
	SessionManager->FindSessions();
}

void UMVE_AUD_AudienceStationModel::SearchRooms(const FString& SearchQuery)
{
	CurrentSearchQuery = SearchQuery;
	PerformSearch();
}

void UMVE_AUD_AudienceStationModel::BindSessionManagerEvents()
{
	if (SessionManager)
	{
		SessionManager->OnSessionsFound.AddUObject(this, &UMVE_AUD_AudienceStationModel::OnSessionsFound);
	}
}

void UMVE_AUD_AudienceStationModel::OnSessionsFound(bool bSuccess, const TArray<FRoomInfo>& Sessions)
{
	if (!bSuccess)
	{
		PRINTLOG(TEXT("Session search failed!"));
		return;
	}

	AllRooms = Sessions;

	if (CurrentSearchQuery.IsEmpty())
	{
		SearchedRooms = AllRooms;
	}
	else
	{
		PerformSearch();
	}

	OnRoomListUpdated.Broadcast(SearchedRooms);
}


void UMVE_AUD_AudienceStationModel::PerformSearch()
{
	SearchedRooms.Empty();

	// 검색어가 비어있으면 전체 목록 표시
	if (CurrentSearchQuery.IsEmpty())
	{
		SearchedRooms = AllRooms;
		PRINTLOG(TEXT("Empty search query - showing all %d rooms"), SearchedRooms.Num());
	}
	else
	{
		// 콘서트 제목 or 방송인 ID로 검색
		for (const FRoomInfo& Room : AllRooms)
		{
			bool bMatch = false;

			// 콘서트 제목에 검색어 포함
			if (Room.RoomTitle.Contains(CurrentSearchQuery))
			{
				bMatch = true;
			}

			// 방송인 ID에 검색어 포함 (RoomID를 방송인 ID로 가정)
			if (Room.RoomID.Contains(CurrentSearchQuery))
			{
				bMatch = true;
			}

			if (bMatch)
			{
				SearchedRooms.Add(Room);
			}
		}

		PRINTLOG(TEXT("Search completed: %d / %d rooms matched"), SearchedRooms.Num(), AllRooms.Num());
	}

	OnRoomListUpdated.Broadcast(SearchedRooms);
}
