
#include "../Public/MVE_GM_StageLevel.h"

#include "MVE.h"
#include "MVE_PC_StageLevel.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AMVE_GM_StageLevel::AMVE_GM_StageLevel()
{
	// OpenLevel로 시작하므로 Seamless Travel 불필요
	bUseSeamlessTravel = false;

	// PlayerController 설정
	PlayerControllerClass = AMVE_PC_StageLevel::StaticClass();
	
	HostCharacterClass = nullptr;
	ClientCharacterClass = nullptr;
}

// 기본 캐릭터 클래스 설정 TODO 나중에 유령 파일 고쳐지면 주석 처리하세용
void AMVE_GM_StageLevel::LoadCharacterClasses()
{
	// Host 캐릭터
	if (!HostCharacterClass)
	{
		FSoftClassPath HostPath(TEXT("/Remocapp/Features/BP_MainCharacter.BP_MainCharacter_C"));
		HostCharacterClass = HostPath.TryLoadClass<APawn>();

		// 디버깅용
		if (HostCharacterClass)
		{
			PRINTLOG(TEXT("Host Character 로드 성공: %s"), *HostCharacterClass->GetName());
		}
		else
		{
			PRINTLOG(TEXT("Host Character 로드 실패!"));
		}
	}

	// Client 캐릭터
	if (!ClientCharacterClass)
	{
		FSoftClassPath ClientPath(TEXT("/Game/Blueprints/Character/BP_ClientCharacter.BP_ClientCharacter_C"));
		ClientCharacterClass = ClientPath.TryLoadClass<APawn>();

		//ㄷㅂㄱ
		if (ClientCharacterClass)
		{
			PRINTLOG(TEXT("Client Character 로드 성공: %s"), *ClientCharacterClass->GetName());
		}
		else
		{
			PRINTLOG(TEXT("Client Character 로드 실패!"));
		}
	}
}

void AMVE_GM_StageLevel::BeginPlay()
{
	Super::BeginPlay();
	
	LoadCharacterClasses();
}

UClass* AMVE_GM_StageLevel::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!HostCharacterClass || !ClientCharacterClass)
		LoadCharacterClasses();
	
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

AActor* AMVE_GM_StageLevel::ChoosePlayerStart_Implementation(AController* Player)
{
	// 호스트/클라이언트 확인
	bool bIsHost = IsHostController(Player);
	FName TargetTag = bIsHost ? FName("Host") : FName("Client");

	PRINTLOG(TEXT("ChoosePlayerStart for %s - Looking for tag: %s"),
		bIsHost ? TEXT("HOST") : TEXT("CLIENT"), *TargetTag.ToString());

	// 해당 태그를 가진 PlayerStart 찾기
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart && PlayerStart->ActorHasTag(TargetTag))
		{
			PRINTLOG(TEXT("Found PlayerStart with tag '%s' at location: %s"),
				*TargetTag.ToString(), *PlayerStart->GetActorLocation().ToString());
			return PlayerStart;
		}
	}

	PRINTLOG(TEXT("No PlayerStart found with tag '%s', using default"), *TargetTag.ToString());

	// 못 찾으면 기본 동작
	return Super::ChoosePlayerStart_Implementation(Player);
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
