
#include "../Public/MVE_GM_StageLevel.h"

#include "MVE.h"
#include "MVE_PC_StageLevel.h"
#include "GameFramework/PlayerController.h"

AMVE_GM_StageLevel::AMVE_GM_StageLevel()
{
	// OpenLevel로 시작하므로 Seamless Travel 불필요
	bUseSeamlessTravel = false;

	// PlayerController 설정
	PlayerControllerClass = AMVE_PC_StageLevel::StaticClass();

	// 기본 캐릭터 클래스는 Blueprint에서 설정
	// HostCharacterClass = nullptr;
	// ClientCharacterClass = nullptr;
}

void AMVE_GM_StageLevel::BeginPlay()
{
	Super::BeginPlay();
}

UClass* AMVE_GM_StageLevel::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 호스트 확인
	if (IsHostController(InController))
	{
		if (HostCharacterClass)
		{
			PRINTLOG(TEXT("Spawning HOST character for controller"));
			return HostCharacterClass;
		}
	}
	else
	{
		if (ClientCharacterClass)
		{
			PRINTLOG(TEXT("Spawning CLIENT character for controller"));
			return ClientCharacterClass;
		}
	}

	// 설정되지 않았으면 기본 캐릭터 사용
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

bool AMVE_GM_StageLevel::IsHostController(AController* Controller) const
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
	{
		return false;
	}

	// 리슨 서버의 첫 번째 로컬 플레이어 = 호스트
	ENetMode NetMode = GetNetMode();

	if (NetMode == NM_ListenServer)
	{
		// 로컬 플레이어인지 확인 (NetConnection이 없으면 로컬 플레이어)
		return PC->IsLocalPlayerController();
	}
	else if (NetMode == NM_Standalone)
	{
		// 스탠드얼론 = 호스트
		return true;
	}

	return false;
}
