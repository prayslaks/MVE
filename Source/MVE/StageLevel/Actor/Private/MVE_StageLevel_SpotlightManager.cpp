// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_StageLevel_SpotlightManager.h"
#include "MVE_StageLevel_Spotlight.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_SpotlightManager::AMVE_StageLevel_SpotlightManager()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_SpotlightManager::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에서 모든 스포트라이트 획득
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMVE_StageLevel_Spotlight::StaticClass(), OutActors);
	for (AActor* Actor : OutActors)
	{
		if (const auto Spotlight = Cast<AMVE_StageLevel_Spotlight>(Actor))
		{
			FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(Spotlight->SequenceOrder);
			Group.SequenceActors.Emplace(Spotlight);
		}
	}
	
	// 모든 시퀀스 순서를 획득해서 정렬
	SequenceActorsBySequenceOrder.GetKeys(SequenceOrders);
	SequenceOrders.Sort();
}