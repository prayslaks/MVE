
#pragma once

#include "CoreMinimal.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMVE, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SessionLogMVE, Log, All);

// 어디서 호출되는지 볼 수 있는 함수
#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

// CALLINFO 매크로 편하게 로그찍는 매크로
#define PRINTINFO() UE_LOG(LogMVE, Warning, TEXT("%s"), *CALLINFO)

// 이거 쓰셈
#define PRINTLOG(fmt, ...) \
UE_LOG(LogMVE, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))

#define SESSIONPRINTLOG(fmt, ...) \
UE_LOG(SessionLogMVE, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))