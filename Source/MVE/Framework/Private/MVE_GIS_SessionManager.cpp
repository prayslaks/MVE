
#include "../Public/MVE_GIS_SessionManager.h"
#include "MVE.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include <string>

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
	if (!SessionInterface.IsValid())
	{
		SESSIONPRINTLOG(TEXT("SessionInterface is invalid"));
		OnSessionCreated.Broadcast(false);
		return;
	}
	
	// 기존 세션이 있는지 확인
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		SESSIONPRINTLOG(TEXT("Existing session found. Destroying before creating new one..."));

		// 세션 생성 정보를 저장하고 대기 플래그 설정
		PendingRoomInfo = RoomInfo;
		bPendingCreateSession = true;

		// 기존 세션 파괴
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	// 기존 세션이 없으면 바로 생성
	CreateSessionInternal(RoomInfo);
}

void UMVE_GIS_SessionManager::CreateSessionInternal(const FRoomInfo& RoomInfo)
{
	if (!SessionInterface.IsValid())
	{
		SESSIONPRINTLOG(TEXT("SessionInterface is invalid"));
		OnSessionCreated.Broadcast(false);
		return;
	}
	
	SESSIONPRINTLOG(TEXT("Creating new session..."));

	// 세션을 만들기 위한 옵션 담을 변수
	FOnlineSessionSettings sessionSettings;
	// 현재 사용중인 서브시스템 이름 가져오자.
	FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
	// 만약에 서브시스템이 이름이 NULL 이면 Lan 을 이용하게 설정
	sessionSettings.bIsLANMatch = subsysName.IsEqual(FName(TEXT("NULL")));
	UE_LOG(LogTemp, Warning, TEXT("서브시스템 : %s"), *subsysName.ToString());

	// Steam 에선 필수 (bUseLobbiesIfAvailable, bUsesPresence)
	// Lobby 사용 여부 설정
	sessionSettings.bUseLobbiesIfAvailable = true;
	// 친구 상태를 확인할 수 있는 여부
	sessionSettings.bUsesPresence = true;

	// 세션 검색 허용 여부
	sessionSettings.bShouldAdvertise = true;
	// 세션 최대 참여 인원 설정
	sessionSettings.NumPublicConnections = RoomInfo.MaxViewers;
	// 세션 중간에 참여 가능 여부 설정
	sessionSettings.bAllowJoinInProgress = true;

	// displayName 을 Base64 로 변환
	FString displayName = StringBase64Encode(RoomInfo.RoomTitle);
	sessionSettings.Set(FName("ROOM_TITLE"), displayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	
	// sessionSettings 이용해서 세션 생성 (NAME_GameSession으로 통일)
	FUniqueNetIdPtr netId =
		GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	SessionInterface->CreateSession(*netId, NAME_GameSession, sessionSettings);
}

APlayerController* UMVE_GIS_SessionManager::GetPlayerController()
{
	return GetGameInstance()->GetWorld()->GetFirstPlayerController();
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

FString UMVE_GIS_SessionManager::StringBase64Encode(FString str)
{
	// str을 std::string 로 변환
	std::string utf8string = TCHAR_TO_UTF8(*str);
	// utf8string 을 uint8 의 Array 변환
	TArray<uint8> arrayData = TArray<uint8>((uint8*)utf8string.c_str(), utf8string.length());

	return FBase64::Encode(arrayData);
}

FString UMVE_GIS_SessionManager::StringBase64Decode(FString str)
{
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)arrayData.GetData(), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

void UMVE_GIS_SessionManager::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	SESSIONPRINTLOG(TEXT("Session creation complete: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

	if (bSuccess)
	{
		SESSIONPRINTLOG(TEXT("Opening StageLevel as new Listen Server..."));
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("StageLevel")), true, TEXT("listen"));
	}

	// 세션 생성 결과 브로드캐스트
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
	SESSIONPRINTLOG(TEXT("Session destroyed: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

	// 세션 생성 대기 중이었다면 이제 생성
	if (bPendingCreateSession && bSuccess)
	{
		SESSIONPRINTLOG(TEXT("Retrying session creation after destroy..."));
		bPendingCreateSession = false;
		CreateSessionInternal(PendingRoomInfo);
	}
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
