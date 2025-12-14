
#include "StageLevel/Default/Public/MVE_GM_StageLevel.h"
#include "MVE.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel_StudioComponent.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel.h"

AMVE_GM_StageLevel::AMVE_GM_StageLevel()
{
	// 심리스 트레블 사용하지 않음
	bUseSeamlessTravel = false;

	// 플레이어 컨트롤러
	if (ConstructorHelpers::FClassFinder<AMVE_PC_StageLevel>
		Finder(TEXT("/Game/Blueprints/Framework/StageLevel/BP_PC_StageLevel.BP_PC_StageLevel_C"));
		Finder.Succeeded())
	{
		PlayerControllerClass = Finder.Class;
	}
	
	// 호스트 캐릭터
	if (ConstructorHelpers::FClassFinder<APawn> 
		Finder(TEXT("/Game/Characters/Features/BP_Orlando.BP_Orlando_C"));
		Finder.Succeeded())
	{
		// 할당
		HostCharacterClass = Finder.Class;
		
		// 디버그
		if (HostCharacterClass)
		{
			PRINTNETLOG(this, TEXT("호스트 캐릭터 로드 성공: %s"), *HostCharacterClass->GetName());
		}
		else
		{
			PRINTNETLOG(this, TEXT("호스트 캐릭터 로드 실패!"));
		}
	}

	// 클라이언트 캐릭터
	if (ConstructorHelpers::FClassFinder<APawn>
		Finder(TEXT("/Game/Workspace/LJW/BP_AudCharacter.BP_AudCharacter_C"));
		Finder.Succeeded())
	{
		// 할당
		ClientCharacterClass = Finder.Class;
		
		// 디버그
		if (ClientCharacterClass)
		{
			PRINTLOG(TEXT("클라이언트 캐릭터 로드 성공: %s"), *ClientCharacterClass->GetName());
		}
		else
		{
			PRINTLOG(TEXT("클라이언트 로드 실패!"));
		}
	}
}

// 콘서트 입장/퇴장 관리

void AMVE_GM_StageLevel::BeginPlay()
{
	Super::BeginPlay();
}

UClass* AMVE_GM_StageLevel::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const bool bIsHost = IsHostController(InController);
	if (UClass* CharacterClass = bIsHost ? HostCharacterClass : ClientCharacterClass)
	{
		PRINTNETLOG(this, TEXT("GetDefaultPawnClassForController_Implementation: %s 컨트롤러를 위해 %s 클래스를 반환"),
			bIsHost ? TEXT("호스트") : TEXT("클라이언트"), *CharacterClass->GetName());
		return CharacterClass;
	}
	
	PRINTNETLOG(this, TEXT("GetDefaultPawnClassForController_Implementation: 적절한 캐릭터 클래스를 찾을 수 없음, 기본 동작 사용."));
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AMVE_GM_StageLevel::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	if (const bool bIsHost = IsHostController(NewPlayer))
	{
		PRINTNETLOG(this, TEXT("새로운 플레이어는 호스트입니다."));
	}
	else
	{
		PRINTNETLOG(this, TEXT("새로운 플레이어는 클라이언트입니다."));
	}
}

void AMVE_GM_StageLevel::Logout(AController* Exiting)
{
	
	
	Super::Logout(Exiting);
}

AActor* AMVE_GM_StageLevel::ChoosePlayerStart_Implementation(AController* Player)
{
	// 호스트/클라이언트 확인
	const bool bIsHost = IsHostController(Player);
	const FName TargetTag = bIsHost ? FName("Host") : FName("Client");

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

// 콘서트 스테이지 음원 관련 RPC

void AMVE_GM_StageLevel::SendPresignedUrlToAllClients(const FString& PresignedUrl)
{
	PRINTNETLOG(this, TEXT("게임모드: 모든 클라이언트에게 Presigned URL 전송: %s"), *PresignedUrl);

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("게임모드: World 또는 GameState가 null이므로 URL을 전송할 수 없습니다."));
		return;
	}

	// 모든 플레이어 상태를 순회하며 RPC 호출
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				// 플레이어 컨트롤러에서 StdComponent를 찾음
				UMVE_PC_StageLevel_StudioComponent* StdComponent = PC->FindComponentByClass<UMVE_PC_StageLevel_StudioComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s에게 URL 전송 중"), *PlayerState->GetPlayerName());
					// 클라이언트에게 오디오 준비를 요청하는 RPC 호출
					StdComponent->Client_PrepareAudio(PresignedUrl);
				}
				else
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s의 StdComponent를 찾을 수 없습니다."), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}

void AMVE_GM_StageLevel::SendPlayCommandToAllClients()
{
	PRINTNETLOG(this, TEXT("게임모드: 모든 클라이언트에게 재생 명령 전송."));

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("게임모드: World 또는 GameState가 null이므로 재생 명령을 전송할 수 없습니다."));
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				UMVE_PC_StageLevel_StudioComponent* StdComponent = PC->FindComponentByClass<UMVE_PC_StageLevel_StudioComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s에게 재생 명령 전송 중"), *PlayerState->GetPlayerName());
					// 클라이언트에게 오디오 재생을 요청하는 RPC 호출
					StdComponent->Client_PlayPreparedAudio();
				}
				else
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s의 StdComponent를 찾을 수 없습니다."), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}

void AMVE_GM_StageLevel::SendStopCommandToAllClients()
{
	PRINTNETLOG(this, TEXT("게임모드: 모든 클라이언트에게 중지 명령 전송."));

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("게임모드: World 또는 GameState가 null이므로 중지 명령을 전송할 수 없습니다."));
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				UMVE_PC_StageLevel_StudioComponent* StdComponent = PC->FindComponentByClass<UMVE_PC_StageLevel_StudioComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s에게 중지 명령 전송 중"), *PlayerState->GetPlayerName());
					// 클라이언트에게 오디오 중지를 요청하는 RPC 호출
					StdComponent->Client_StopPreparedAudio();
				}
				else
				{
					PRINTNETLOG(this, TEXT("게임모드: 플레이어 %s의 StdComponent를 찾을 수 없습니다."), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}