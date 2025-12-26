// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_StageLevel_SpotlightManager.h"

#include "MVE_StageLevel_Spotlight.h"
#include "Kismet/GameplayStatics.h"

AMVE_StageLevel_SpotlightManager::AMVE_StageLevel_SpotlightManager()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = true;
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
			FSpotlightGroup& Group = SpotlightsBySequenceOrder.FindOrAdd(Spotlight->SequenceOrder);
			Group.Spotlights.Emplace(Spotlight);
		}
	}
	
	// 모든 시퀀스 순서를 획득해서 정렬
	SpotlightsBySequenceOrder.GetKeys(SequenceOrders);
	SequenceOrders.Sort();
}

void AMVE_StageLevel_SpotlightManager::ExecuteSequence(int32 SequenceNumber, const float DelayBetweenOrder)
{
	int32 CurrentIndex = 0;
	FTimerDelegate TimerDelegate;
	GetWorldTimerManager().SetTimer(ExecutionDelayTimerHandle, TimerDelegate.CreateLambda([SequenceNumber, CurrentIndex, this]()
	{
		FSpotlightGroup& Group = SpotlightsBySequenceOrder.FindOrAdd(CurrentIndex);
		for (const auto Spotlight : Group.Spotlights)
		{
			Spotlight->ExecuteSequence(SequenceNumber);
		}
	}), DelayBetweenOrder, false);
}