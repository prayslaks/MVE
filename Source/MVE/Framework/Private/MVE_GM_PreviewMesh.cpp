#include "../Public/MVE_GM_PreviewMesh.h"
#include "MVE.h"
#include "MVE_AUD_PreviewCameraPawn.h"
#include "MVE_PC_PreviewMesh.h"

AMVE_GM_PreviewMesh::AMVE_GM_PreviewMesh()
{
	// DefaultPawn은 Blueprint에서 설정 (PointerInteraction 상속 BP_PreviewCameraPawn)
	// DefaultPawnClass = nullptr; // 에디터에서 설정하도록 비워둠

	// PlayerController 설정
	PlayerControllerClass = AMVE_PC_PreviewMesh::StaticClass();
}

void AMVE_GM_PreviewMesh::BeginPlay()
{
	Super::BeginPlay();
}

void AMVE_GM_PreviewMesh::StartPlay()
{
	Super::StartPlay();

	// 프리뷰 캐릭터 스폰
	SpawnPreviewCharacter();
}

void AMVE_GM_PreviewMesh::SpawnPreviewCharacter()
{
	if (!PreviewCharacterClass)
	{
		PRINTLOG(TEXT("❌ PreviewCharacterClass is not set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewCharacter = World->SpawnActor<AActor>(
		PreviewCharacterClass,
		CharacterSpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (PreviewCharacter)
	{
		PRINTLOG(TEXT("✅ Preview Character spawned: %s at %s"),
			*PreviewCharacter->GetName(),
			*CharacterSpawnLocation.ToString());
	}
	else
	{
		PRINTLOG(TEXT("❌ Failed to spawn Preview Character"));
	}
}
