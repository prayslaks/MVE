#include "MVE_StageLevel_FlameManager.h"

#include "MVE_StageLevel_Flame.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_FlameManager::AMVE_StageLevel_FlameManager()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_FlameManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에서 모든 스포트라이트 획득
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_Flame::StaticClass(), OutActors);
	for (AActor* Actor : OutActors)
	{
		if (const auto Flame = Cast<AMVE_StageLevel_Flame>(Actor))
		{
			FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(Flame->SequenceOrder);
			Group.SequenceActors.Emplace(Flame);
		}
	}
	
	// 모든 시퀀스 순서를 획득해서 정렬
	SequenceActorsBySequenceOrder.GetKeys(SequenceOrders);
	SequenceOrders.Sort();
}