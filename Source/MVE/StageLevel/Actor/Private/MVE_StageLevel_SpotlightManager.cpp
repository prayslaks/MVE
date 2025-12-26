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
			FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(Spotlight->SequenceOrder);
			Group.SequenceActors.Emplace(Spotlight);
		}
	}
	
	// 모든 시퀀스 순서를 획득해서 정렬
	SequenceActorsBySequenceOrder.GetKeys(SequenceOrders);
	SequenceOrders.Sort();
}

void AMVE_StageLevel_SpotlightManager::ExecuteSequenceNumber(const int32 SequenceNumber, const float DelayBetweenOrder)
{
	CurrentSequenceOrderIndex = 0;
	CurrentSequenceNumber = SequenceNumber;
	
	// 딜레이가 0.09초 이상이면
	if (DelayBetweenOrder > 0.09)
	{
		// 시퀀스 오더에 따라서 딜레이를 두고 순차 실행
		FTimerDelegate TimerDelegate;
		GetWorldTimerManager().SetTimer(ExecutionDelayTimerHandle, TimerDelegate.CreateLambda([this]()
		{
			ExecuteSequenceActorByOrder();
		}), DelayBetweenOrder, true);	
	}
	else
	{
		// 시퀀스 오더를 무시하고 일괄 실행
		for (int32 i = 0; i < SequenceActorsBySequenceOrder.Num(); i++)
		{
			const int32 CurrentSequenceOrder = SequenceOrders[i];
			FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(CurrentSequenceOrder);
			for (const auto Spotlight : Group.SequenceActors)
			{
				Spotlight->ExecuteSequence(CurrentSequenceNumber);
			}
		}
	}
}

void AMVE_StageLevel_SpotlightManager::ExecuteSequenceActorByOrder()
{
	// 시퀀스 오더 인덱스가 시퀀스 오더 배열의 크기를 초과
	if (CurrentSequenceOrderIndex >= SequenceOrders.Num())
	{
		GetWorldTimerManager().ClearTimer(ExecutionDelayTimerHandle);
		return;
	}
	
	// 이번 시퀀스 오더에 해당하는 모든 스포트라이트에 시퀀스 넘버 전달 
	const int32 CurrentSequenceOrder = SequenceOrders[CurrentSequenceOrderIndex];
	FSequenceActorGroup& Group = SequenceActorsBySequenceOrder.FindOrAdd(CurrentSequenceOrder);
	for (const auto Spotlight : Group.SequenceActors)
	{
		Spotlight->ExecuteSequence(CurrentSequenceNumber);
	}
	
	// 다음 시퀀스 오더 인덱스
	CurrentSequenceOrderIndex++;
}