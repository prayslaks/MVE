#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMVE, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SessionLogMVE, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(ChattingLogMVE, Log, All);

// Helper function to get the network mode as a string from a UObject context
FORCEINLINE FString GetNetModeString(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return FString(); // Return empty string if no context
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return FString(); // Return empty string if no world
	}

	switch (World->GetNetMode())
	{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			// In PIE, GPlayInEditorID is > 0 for clients. This helps distinguish them.
			return FString::Printf(TEXT("Client_%d"), GPlayInEditorID > 0 ? GPlayInEditorID - 1 : 0);
		default:
			return FString(); // Return empty string for unknown modes
	}
}

// 호출 위치
#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

// 호출 위치가 포함된 로그
#define PRINTINFO() UE_LOG(LogMVE, Warning, TEXT("%s"), *CALLINFO)

// 호출 위치가 포함된 포매팅
#define PRINTLOG(fmt, ...) \
UE_LOG(LogMVE, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))

#define SESSIONPRINTLOG(fmt, ...) \
UE_LOG(SessionLogMVE, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))

#define PRINTLOG_CHAT(fmt, ...) \
UE_LOG(ChattingLogMVE, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))


// 넷모드 정보를 포함한 로그 포매팅 (PRINTNETLOG)
// 사용법: PRINTNETLOG(this, TEXT("로그 메시지"));
#define PRINTNETLOG(ContextObject, fmt, ...) \
	UE_LOG(LogMVE, Warning, TEXT("[%s] %s : %s"), *GetNetModeString(ContextObject), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))