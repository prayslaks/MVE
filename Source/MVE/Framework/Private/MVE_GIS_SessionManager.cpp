
#include "../Public/MVE_GIS_SessionManager.h"
#include "OnlineSessionSettings.h"
#include "MVE.h"
#include "Online/OnlineSessionNames.h"

void UMVE_GIS_SessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Online Subsystem 가져오기
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
        
		if (SessionInterface.IsValid())
		{
			// 델리게이트 바인딩
			CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
				FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMVE_GIS_SessionManager::OnCreateSessionComplete));
            
			FindSessionsDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
				FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMVE_GIS_SessionManager::OnFindSessionsComplete));
            
			JoinSessionDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMVE_GIS_SessionManager::OnJoinSessionComplete));
            
			DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMVE_GIS_SessionManager::OnDestroySessionComplete));
			LeaveSessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
							FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMVE_GIS_SessionManager::OnLeaveSessionComplete));
			
			PRINTLOG(TEXT("SessionManager initialized"));
		}
	}
}

void UMVE_GIS_SessionManager::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionDelegateHandle);
	}
	
	Super::Deinitialize();
}

void UMVE_GIS_SessionManager::CreateSession(const FRoomInfo& RoomInfo)
{
	// 세션 설정
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
    
	SessionSettings->NumPublicConnections = RoomInfo.MaxViewers;
	SessionSettings->bShouldAdvertise = true; // 검색 가능
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;

	// 커스텀 데이터 설정
	SessionSettings->Set(FName("ROOM_TITLE"), RoomInfo.RoomTitle, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(FName("CONCERT_TYPE"), RoomInfo.ConcertType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(FName("IS_LIVE"), RoomInfo.bIsLive, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	

	// 세션 생성
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void UMVE_GIS_SessionManager::FindSessions()
{
	if (!SessionInterface.IsValid())
	{
		PRINTLOG(TEXT("SessionInterface is invalid!"));
		OnSessionsFound.Broadcast(false, TArray<FRoomInfo>());
		return;
	}

	// 세션 검색 설정
	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 100;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	PRINTLOG(TEXT("Searching for sessions..."));

	// 세션 검색 시작
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void UMVE_GIS_SessionManager::JoinSession(const FString& SessionId)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		PRINTLOG(TEXT("Cannot join session - invalid interface or search"));
		OnSessionJoined.Broadcast(false);
		return;
	}

	// SessionId로 검색 결과에서 해당 세션 찾기
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		FString ResultSessionId = SearchResult.GetSessionIdStr();
        
		if (ResultSessionId == SessionId)
		{
			PRINTLOG(TEXT("Joining session: %s"), *SessionId);

			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult);
			return;
		}
	}

	PRINTLOG(TEXT("Session not found: %s"), *SessionId);
	OnSessionJoined.Broadcast(false);
}

void UMVE_GIS_SessionManager::LeaveSession()
{
	SessionInterface->DestroySession(NAME_GameSession);
}

void UMVE_GIS_SessionManager::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->DestroySession(NAME_GameSession);
}

void UMVE_GIS_SessionManager::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	OnSessionCreated.Broadcast(bSuccess);

	if (bSuccess)
	{
		PRINTLOG(TEXT("Starting listen server..."));
        
		// 리슨 서버로 방송 맵 이동
		// ?listen 옵션이 리슨 서버를 활성화합니다
		UWorld* World = GetWorld();
		if (World)
		{
			// 방송용 맵으로 서버 이동
			// "/Game/Maps/Studio/StudioBroadcastMap" 같은 경로를 사용
			World->ServerTravel("/Game/Maps/ConcertMap?listen'");
		}
	}

	OnSessionCreated.Broadcast(bSuccess);
}

void UMVE_GIS_SessionManager::OnFindSessionsComplete(bool bSuccess)
{
	if (!bSuccess || !SessionSearch.IsValid())
	{
		OnSessionsFound.Broadcast(false, TArray<FRoomInfo>());
		return;
	}

	TArray<FRoomInfo> FoundRooms;

	PRINTLOG(TEXT("Found %d sessions"), SessionSearch->SearchResults.Num());

	// 검색 결과를 FRoomInfo로 변환
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		FRoomInfo RoomInfo = ConvertSearchResultToRoomInfo(SearchResult);
		FoundRooms.Add(RoomInfo);

		PRINTLOG(TEXT("Session: %s (Ping: %d)"), 
				 *RoomInfo.RoomTitle, SearchResult.PingInMs);
	}

	OnSessionsFound.Broadcast(true, FoundRooms);
}

void UMVE_GIS_SessionManager::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
	OnSessionJoined.Broadcast(bSuccess);

	if (bSuccess && SessionInterface.IsValid())
	{
		// 서버 주소 가져오기
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			PRINTLOG(TEXT("Connecting to: %s"), *ConnectInfo);

			// 서버로 이동
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMVE_GIS_SessionManager::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	PRINTLOG(TEXT("Session destroyed: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
}

FRoomInfo UMVE_GIS_SessionManager::ConvertSearchResultToRoomInfo(const FOnlineSessionSearchResult& SearchResult)
{
	FRoomInfo RoomInfo;

	// 세션 ID
	RoomInfo.RoomID = SearchResult.GetSessionIdStr();

	// 커스텀 데이터 가져오기
	FString RoomTitle;
	if (SearchResult.Session.SessionSettings.Get(FName("ROOM_TITLE"), RoomTitle))
	{
		RoomInfo.RoomTitle = RoomTitle;
	}

	FString ConcertType;
	if (SearchResult.Session.SessionSettings.Get(FName("CONCERT_TYPE"), ConcertType))
	{
		RoomInfo.ConcertType = ConcertType;
	}

	bool bIsLive = false;
	if (SearchResult.Session.SessionSettings.Get(FName("IS_LIVE"), bIsLive))
	{
		RoomInfo.bIsLive = bIsLive;
	}

	// 시청자 수
	RoomInfo.ViewerCount = SearchResult.Session.SessionSettings.NumPublicConnections - 
						   SearchResult.Session.NumOpenPublicConnections;

	return RoomInfo;
}

void UMVE_GIS_SessionManager::OnLeaveSessionComplete(FName SessionName, bool bSuccess)
{
	PRINTLOG(TEXT("Leave session result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

	if (bSuccess)
	{
		// 세션 나간 후 로비로 이동 등 추가 로직
		PRINTLOG(TEXT("Successfully left the session"));
	}
}
