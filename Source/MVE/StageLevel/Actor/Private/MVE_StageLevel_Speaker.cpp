#include "StageLevel/Actor/Public/MVE_StageLevel_Speaker.h"

#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "MVE.h"

AMVE_StageLevel_Speaker::AMVE_StageLevel_Speaker()
{
	// 틱 활성화
	PrimaryActorTick.bCanEverTick = true;
	
	// 리플리케이션 활성화
	bReplicates = true;
	
	// 메시 컴포넌트 추가
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	// 큐브를 기본 메시로 설정
	if (static ConstructorHelpers::FObjectFinder<UStaticMesh> 
		Finder(TEXT("/Engine/BasicShapes/Cube"));
		Finder.Succeeded())
	{
		MeshComponent->SetStaticMesh(Finder.Object);	
	}

	// 오디오 컴포넌트 추가
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
}

void AMVE_StageLevel_Speaker::BeginPlay()
{
	Super::BeginPlay();
	PRINTNETLOG(this, TEXT("Speaker BeginPlay 완료"));
}

void AMVE_StageLevel_Speaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}