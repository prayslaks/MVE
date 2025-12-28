#include "MVE_StageLevel_FanfareManager.h"

#include "MVE_StageLevel_Fanfare.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_FanfareManager::AMVE_StageLevel_FanfareManager()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_FanfareManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에서 모든 스포트라이트 획득
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_Fanfare::StaticClass(), OutActors);
	for (AActor* Actor : OutActors)
	{
		if (const auto Fanfare = Cast<AMVE_StageLevel_Fanfare>(Actor))
		{
			FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(Fanfare->SequenceOrder);
			Group.SequenceActors.Emplace(Fanfare);
		}
	}
	
	// 모든 시퀀스 순서를 획득해서 정렬
	SequenceActorsBySequenceOrder.GetKeys(SequenceOrders);
	SequenceOrders.Sort();
}