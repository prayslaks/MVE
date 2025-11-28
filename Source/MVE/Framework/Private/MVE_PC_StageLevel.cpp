
#include "../Public/MVE_PC_StageLevel.h"

#include "MVE.h"
#include "UIManagerSubsystem.h"

void AMVE_PC_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 UI 표시
	if (!IsLocalPlayerController())
	{
		PRINTLOG(TEXT("Not local player - skipping UI"));
		return;
	}

	// 호스트/클라이언트 구분
	bool bIsHost = HasAuthority() && IsListenServerHost();
    
	if (bIsHost)
	{
		ShowStudioUI();
	}
	else
	{
		ShowAudienceUI();
	}
}

void AMVE_PC_StageLevel::ShowStudioUI()
{
	UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
    
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::StudioBroadcast);
	}
}

void AMVE_PC_StageLevel::ShowAudienceUI()
{
	UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
    
	if (UIManager)
	{
		UIManager->ShowScreen(EUIScreen::AudienceGenerateMesh);
	}
}

bool AMVE_PC_StageLevel::IsListenServerHost() const
{
	ENetMode NetMode = GetNetMode();
    
	if (NetMode == NM_ListenServer)
	{
		// 리슨 서버에서 로컬 플레이어 = 호스트
		return IsLocalPlayerController();
	}
	else if (NetMode == NM_Client)
	{
		// 일반 클라이언트
		return false;
	}
	else if (NetMode == NM_Standalone)
	{
		// 스탠드얼론 (싱글플레이) - 호스트로 간주
		return true;
	}

	return false;
}
