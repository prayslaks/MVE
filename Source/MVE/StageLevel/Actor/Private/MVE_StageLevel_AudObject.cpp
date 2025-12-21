#include "StageLevel/Actor/Public/MVE_StageLevel_AudObject.h"

#include "Components/BoxComponent.h"

AMVE_StageLevel_AudObject::AMVE_StageLevel_AudObject()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
	
	// 충돌체 설정
	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetCollisionProfileName(FName("NoCollision"));
}

void AMVE_StageLevel_AudObject::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 가시성 설정
	bIsVisible = false;
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