#include "StageLevel/Actor/Public/MVE_StageLevel_AudObject.h"

AMVE_StageLevel_AudObject::AMVE_StageLevel_AudObject()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_AudObject::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 가시성 설정
	StaticMeshComp->SetVisibility(bIsVisible);
}

void AMVE_StageLevel_AudObject::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// 에디터에서 bIsVisible 값 변경 시 즉시 반영
	StaticMeshComp->SetVisibility(bIsVisible);
}

void AMVE_StageLevel_AudObject::SetIsVisible(const bool Value)
{
	bIsVisible = Value;
	StaticMeshComp->SetVisibility(bIsVisible);
}