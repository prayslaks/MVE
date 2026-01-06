// Copyright Epic Games, Inc. All Rights Reserved.

#include "MVE.h"
#include "Modules/ModuleManager.h"
#include "HAL/IConsoleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, MVE, "MVE" );

DEFINE_LOG_CATEGORY(LogMVE)
DEFINE_LOG_CATEGORY(SessionLogMVE)
DEFINE_LOG_CATEGORY(ChattingLogMVE)

// 1. CVar 선언
static TAutoConsoleVariable<int32> CVarTestMode(
	TEXT("myproject.TestMode"),				// 콘솔창에서 입력할 명령 이름
	0,										// 기본값
	TEXT("0: API 모드, 1: 더미 데이터 모드"),	// 도움말 설명
	ECVF_Default							// 플래그 (치트 방지 등을 설정 가능)
);