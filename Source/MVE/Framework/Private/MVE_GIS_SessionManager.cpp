
#include "../Public/MVE_GIS_SessionManager.h"
#include "MVE.h"
#include "MVE_API_Helper.h"


// ========================================
// 세션 생성
// ========================================

void UMVE_GIS_SessionManager::CreateSession(const FConcertInfo& ConcertInfo)
{
	SESSIONPRINTLOG(TEXT("Creating concert session: %s"), *ConcertInfo.ConcertName);
    
	bIsHost = true;
	PendingConcertInfo = ConcertInfo;
    
	CreateConcertInternal(ConcertInfo);
}

void UMVE_GIS_SessionManager::CreateConcertInternal(const FConcertInfo& ConcertInfo)
{
	FOnCreateConcertComplete OnResult;
	OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnCreateConcertComplete);
    
	// FConcertInfo에서 필요한 필드만 API로 전달
	UMVE_API_Helper::CreateConcert(
		ConcertInfo.ConcertName,
		ConcertInfo.Songs,
		ConcertInfo.Accessories,
		ConcertInfo.MaxAudience,
		OnResult
	);
}

void UMVE_GIS_SessionManager::OnCreateConcertComplete(bool bSuccess, const FConcertCreationData& Data, const FString& ErrorCode)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("CreateConcert failed: %s"), *ErrorCode);
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	CurrentRoomId = Data.RoomId;
	SESSIONPRINTLOG(TEXT("Concert created. RoomId: %s"), *CurrentRoomId);
    
	// Step 2: Listen Server 시작
	UWorld* World = GetWorld();
	if (!World)
	{
		SESSIONPRINTLOG(TEXT("World is null!"));
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	// ServerTravel로 Listen Server 시작
	FString TravelURL = TEXT("/Game/Maps/StageLevel?listen");
	World->ServerTravel(TravelURL);
    
	// ServerTravel 완료 후 RegisterListenServer 호출
	// (약간의 지연 필요 - Listen Server가 완전히 시작될 때까지 대기)
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UMVE_GIS_SessionManager::RegisterListenServer,
		2.0f, // 2초 대기
		false
	);
}

void UMVE_GIS_SessionManager::RegisterListenServer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		SESSIONPRINTLOG(TEXT("World is null during RegisterListenServer!"));
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	FString LocalIP;
	int32 Port;
    
	if (!UMVE_API_Helper::GetHostLocalIPAndPort(World, LocalIP, Port))
	{
		SESSIONPRINTLOG(TEXT("Failed to get local IP and port"));
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Registering Listen Server: %s:%d"), *LocalIP, Port);
    
	FOnRegisterListenServerComplete OnResult;
	OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnRegisterListenServerComplete);
    
	UMVE_API_Helper::RegisterListenServer(
		CurrentRoomId,
		LocalIP,
		Port,
		TEXT(""), // PublicIP (옵션)
		0,        // PublicPort (옵션)
		OnResult
	);
}

void UMVE_GIS_SessionManager::OnRegisterListenServerComplete(bool bSuccess, const FRegisterListenServerData& Data, const FString& ErrorCode)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("RegisterListenServer failed: %s"), *ErrorCode);
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Listen Server registered successfully"));

	SESSIONPRINTLOG(TEXT("Session created. Waiting for user to start broadcast..."));
	OnSessionCreated.Broadcast(true); // 세션 생성 완료!
}

void UMVE_GIS_SessionManager::OpenConcert()
{
	FOnGenericApiComplete OnResult;
	OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnToggleConcertOpenComplete);
    
	UMVE_API_Helper::ToggleConcertOpen(CurrentRoomId, true, OnResult);
}

void UMVE_GIS_SessionManager::OnToggleConcertOpenComplete(bool bSuccess, const FString& MessageOrError)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("ToggleConcertOpen failed: %s"), *MessageOrError);
		OnSessionCreated.Broadcast(false);
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Concert opened successfully. RoomId: %s"), *CurrentRoomId);
    
	// 세션 생성 완료!
	OnSessionCreated.Broadcast(true);
}


// ========================================
// 세션 목록 조회
// ========================================


void UMVE_GIS_SessionManager::FindSessions()
{
	SESSIONPRINTLOG(TEXT("Finding concert sessions..."));
    
	ConcertList.Empty(); // 기존 목록 초기화
    
	FOnGetConcertListComplete OnResult;
	OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnGetConcertListComplete);
    
	UMVE_API_Helper::GetConcertList(OnResult);
}

void UMVE_GIS_SessionManager::OnGetConcertListComplete(bool bSuccess, const FGetConcertListData& Data, const FString& ErrorCode)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("GetConcertList failed: %s"), *ErrorCode);
		OnSessionsFound.Broadcast(false, 0);
		return;
	}
    
	ConcertList = Data.Concerts;
	SESSIONPRINTLOG(TEXT("Found %d concerts"), ConcertList.Num());
    
	// 세션 목록 조회 완료
	OnSessionsFound.Broadcast(true, ConcertList.Num());
}

bool UMVE_GIS_SessionManager::GetConcertInfoAtIndex(int32 Index, FConcertInfo& OutConcertInfo) const
{
	if (!ConcertList.IsValidIndex(Index))
	{
		return false;
	}
    
	OutConcertInfo = ConcertList[Index];
	return true;
}

FString UMVE_GIS_SessionManager::GetCurrentSessionName()
{
	return PendingConcertInfo.ConcertName;
}

FString UMVE_GIS_SessionManager::GetCurrentRoomID()
{
	return CurrentRoomId;
}


// ========================================
// 방송 시작/종료
// ========================================

void UMVE_GIS_SessionManager::StartBroadcast()
{
	if (CurrentRoomId.IsEmpty())
	{
		SESSIONPRINTLOG(TEXT("Cannot start broadcast: No active session"));
		return;
	}
    
	if (!bIsHost)
	{
		SESSIONPRINTLOG(TEXT("Cannot start broadcast: Not a host"));
		return;
	}
    
	if (bIsBroadcasting)
	{
		SESSIONPRINTLOG(TEXT("Broadcast already started"));
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Starting broadcast for RoomId: %s"), *CurrentRoomId);
    
	FOnGenericApiComplete OnResult;
	OnResult.BindLambda([this](bool bSuccess, FString MessageOrError)
	{
		if (!bSuccess)
		{
			SESSIONPRINTLOG(TEXT("StartBroadcast failed: %s"), *MessageOrError);
			OnBroadcastStateChanged.Broadcast(false);
			return;
		}
        
		bIsBroadcasting = true;
		SESSIONPRINTLOG(TEXT("Broadcast started successfully. Concert is now visible to viewers."));
		OnBroadcastStateChanged.Broadcast(true);
	});
    
	UMVE_API_Helper::ToggleConcertOpen(CurrentRoomId, true, OnResult);
}

void UMVE_GIS_SessionManager::StopBroadcast()
{
	if (CurrentRoomId.IsEmpty())
	{
		SESSIONPRINTLOG(TEXT("Cannot stop broadcast: No active session"));
		return;
	}
    
	if (!bIsHost)
	{
		SESSIONPRINTLOG(TEXT("Cannot stop broadcast: Not a host"));
		return;
	}
    
	if (!bIsBroadcasting)
	{
		SESSIONPRINTLOG(TEXT("Broadcast already stopped"));
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Stopping broadcast for RoomId: %s"), *CurrentRoomId);
    
	FOnGenericApiComplete OnResult;
	OnResult.BindLambda([this](bool bSuccess, FString MessageOrError)
	{
		if (!bSuccess)
		{
			SESSIONPRINTLOG(TEXT("StopBroadcast failed: %s"), *MessageOrError);
			return;
		}
        
		bIsBroadcasting = false;
		SESSIONPRINTLOG(TEXT("Broadcast stopped. Concert is no longer visible to viewers."));
		OnBroadcastStateChanged.Broadcast(false);
	});
    
	UMVE_API_Helper::ToggleConcertOpen(CurrentRoomId, false, OnResult);
}

// ========================================
// 세션 참가
// ========================================

void UMVE_GIS_SessionManager::JoinSession(int32 Index)
{
	if (!ConcertList.IsValidIndex(Index))
	{
		SESSIONPRINTLOG(TEXT("Invalid concert index: %d"), Index);
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	const FConcertInfo& ConcertInfo = ConcertList[Index];
	JoinConcertInternal(ConcertInfo.RoomId);
}

void UMVE_GIS_SessionManager::JoinSessionByRoomId(const FString& RoomId)
{
	if (RoomId.IsEmpty())
	{
		SESSIONPRINTLOG(TEXT("RoomId is empty!"));
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	JoinConcertInternal(RoomId);
}

void UMVE_GIS_SessionManager::JoinConcertInternal(const FString& RoomId)
{
	CurrentRoomId = RoomId;
	bIsHost = false;
    
	SESSIONPRINTLOG(TEXT("Joining concert: %s"), *RoomId);
    
	// Step 1: Concert 상세 정보 조회 (IP:Port 확인)
	FOnGetConcertInfoComplete OnGetInfoResult;
	OnGetInfoResult.BindUObject(this, &UMVE_GIS_SessionManager::OnGetConcertInfoForJoin);
    
	UMVE_API_Helper::GetConcertInfo(RoomId, OnGetInfoResult);
}

void UMVE_GIS_SessionManager::OnGetConcertInfoForJoin(bool bSuccess, const FGetConcertInfoResponseData& Data,
	const FString& ErrorCode)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("GetConcertInfo failed: %s"), *ErrorCode);
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	// FConcertInfo의 ListenServer 정보 추출
	const FListenServer& ListenServer = Data.Concert.ListenServer;
    
	// LocalIP와 Port 확인 (PublicIP가 있으면 우선 사용)
	FString ServerIP = ListenServer.PublicIP.IsEmpty() ? ListenServer.LocalIP : ListenServer.PublicIP;
	int32 ServerPort = ListenServer.PublicPort > 0 ? ListenServer.PublicPort : ListenServer.Port;
    
	if (ServerIP.IsEmpty() || ServerPort <= 0)
	{
		SESSIONPRINTLOG(TEXT("Listen Server info not available. IP: %s, Port: %d"), 
			*ServerIP, ServerPort);
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	PendingTravelIP = ServerIP;
	PendingTravelPort = ServerPort;
    
	SESSIONPRINTLOG(TEXT("Concert info received. Server: %s:%d"), *PendingTravelIP, PendingTravelPort);
    
	// Step 2: Redis에 참가 등록
	FOnGenericApiComplete OnJoinResult;
	OnJoinResult.BindUObject(this, &UMVE_GIS_SessionManager::OnJoinConcertComplete);
    
	int32 ClientId = GetUniqueClientId();
	UMVE_API_Helper::JoinConcert(CurrentRoomId, ClientId, OnJoinResult);
}

void UMVE_GIS_SessionManager::OnJoinConcertComplete(bool bSuccess, const FString& MessageOrError)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("JoinConcert failed: %s"), *MessageOrError);
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Joined concert successfully in Redis"));
    
	// Step 3: 실제 게임 서버로 이동
	UWorld* World = GetWorld();
	if (!World)
	{
		SESSIONPRINTLOG(TEXT("World is null!"));
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		SESSIONPRINTLOG(TEXT("PlayerController is null!"));
		OnSessionJoined.Broadcast(false);
		return;
	}
    
	FString TravelURL = FString::Printf(TEXT("%s:%d"), *PendingTravelIP, PendingTravelPort);
	SESSIONPRINTLOG(TEXT("Traveling to: %s"), *TravelURL);
    
	PC->ClientTravel(TravelURL, TRAVEL_Absolute);
    
	// 참가 완료!
	OnSessionJoined.Broadcast(true);
}

// ========================================
// 세션 종료
// ========================================

void UMVE_GIS_SessionManager::DestroySession()
{
	if (CurrentRoomId.IsEmpty())
	{
		SESSIONPRINTLOG(TEXT("No active session to destroy"));
		return;
	}
    
	SESSIONPRINTLOG(TEXT("Destroying session: %s (Host: %d)"), *CurrentRoomId, bIsHost);
    
	if (bIsHost)
	{
		// 호스트: Concert 닫기
		FOnGenericApiComplete OnResult;
		OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnCloseConcertComplete);
        
		UMVE_API_Helper::ToggleConcertOpen(CurrentRoomId, false, OnResult);
	}
	else
	{
		// 클라이언트: Concert 떠나기
		FOnGenericApiComplete OnResult;
		OnResult.BindUObject(this, &UMVE_GIS_SessionManager::OnLeaveConcertComplete);
        
		int32 ClientId = GetUniqueClientId();
		UMVE_API_Helper::LeaveConcert(CurrentRoomId, ClientId, OnResult);
	}
}

void UMVE_GIS_SessionManager::OnLeaveConcertComplete(bool bSuccess, const FString& MessageOrError)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("LeaveConcert failed: %s"), *MessageOrError);
	}
	else
	{
		SESSIONPRINTLOG(TEXT("Left concert successfully"));
	}
    
	// 성공/실패 관계없이 메인 레벨로 복귀
	CurrentRoomId.Empty();
	bIsHost = false;
    
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->ClientTravel(TEXT("/Game/Maps/MainLevel"), TRAVEL_Absolute);
		}
	}
}

void UMVE_GIS_SessionManager::OnCloseConcertComplete(bool bSuccess, const FString& MessageOrError)
{
	if (!bSuccess)
	{
		SESSIONPRINTLOG(TEXT("CloseConcert failed: %s"), *MessageOrError);
	}
	else
	{
		SESSIONPRINTLOG(TEXT("Closed concert successfully"));
	}
    
	// 성공/실패 관계없이 메인 레벨로 복귀
	CurrentRoomId.Empty();
	bIsHost = false;
    
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->ClientTravel(TEXT("/Game/Maps/MainLevel"), TRAVEL_Absolute);
		}
	}
}

// ========================================
// 유틸리티
// ========================================

int32 UMVE_GIS_SessionManager::GetUniqueClientId() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return -1;
	}
    
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return -1;
	}
    
	// NetPlayerIndex 사용 (멀티플레이어에서 고유 ID)
	return PC->NetPlayerIndex;
}
