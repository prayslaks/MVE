
#include "MVE_GISubsystem_SessionManager.h"

void UMVE_GISubsystem_SessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMVE_GISubsystem_SessionManager::Deinitialize()
{
	Super::Deinitialize();
}

void UMVE_GISubsystem_SessionManager::CreateSession(const FRoomInfo& RoomInfo)
{
}

void UMVE_GISubsystem_SessionManager::FindSessions()
{
}

void UMVE_GISubsystem_SessionManager::JoinSession(const FString& SessionId)
{
}

void UMVE_GISubsystem_SessionManager::LeaveSession()
{
}

void UMVE_GISubsystem_SessionManager::DestroySession()
{
}

void UMVE_GISubsystem_SessionManager::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
}

void UMVE_GISubsystem_SessionManager::OnFindSessionsComplete(bool bSuccess)
{
}

void UMVE_GISubsystem_SessionManager::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
}

void UMVE_GISubsystem_SessionManager::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
}

FRoomInfo UMVE_GISubsystem_SessionManager::ConvertSearchResultToRoomInfo(const FOnlineSessionSearchResult& SearchResult)
{
	return FRoomInfo();
}
