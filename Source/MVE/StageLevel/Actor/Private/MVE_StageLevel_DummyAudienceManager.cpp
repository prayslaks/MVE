#include "MVE_StageLevel_DummyAudienceManager.h"

#include "MVE_StageLevel_DummyAudience.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_DummyAudienceManager::AMVE_StageLevel_DummyAudienceManager()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_DummyAudienceManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에 존재하는 모든 더미 오디언스 획득
	TArray<AActor*> FindActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMVE_StageLevel_DummyAudience::StaticClass(), FindActors);
	DummyAudiences.Reserve(FindActors.Num());
	Algo::Transform(FindActors, DummyAudiences, [](AActor* InActor)
	{
		return CastChecked<AMVE_StageLevel_DummyAudience>(InActor);
	});
	DummyAudiences.Remove(nullptr);
}

void AMVE_StageLevel_DummyAudienceManager::OnPlayMusic_Implementation()
{
	for (const auto DummyAudience : DummyAudiences)
	{
		DummyAudience->SetIsDancing(true);
	}
}

void AMVE_StageLevel_DummyAudienceManager::OnStopMusic_Implementation()
{
	for (const auto DummyAudience : DummyAudiences)
	{
		DummyAudience->SetIsDancing(false);
	}
}