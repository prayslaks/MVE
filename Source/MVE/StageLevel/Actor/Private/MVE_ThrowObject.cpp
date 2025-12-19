// Fill out your copyright notice in the Description page of Project Settings.


#include "StageLevel/Actor/Public/MVE_ThrowObject.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMVE_ThrowObject::AMVE_ThrowObject()
{
	//틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	// 충돌체
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetCollisionProfileName(FName("BlockAllDynamic"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	
	// 메시
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

	// 발사체 컴포넌트
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->SetUpdatedComponent(RootComponent);
	ProjectileMovementComp->InitialSpeed = 3000.f;
	ProjectileMovementComp->MaxSpeed = 3000.f;
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->bShouldBounce = true;
	ProjectileMovementComp->ProjectileGravityScale = 0.5f;
	ProjectileMovementComp->SetActive(false);
}

void AMVE_ThrowObject::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버만 물리 효과를 연산하고, 클라이언트에 리플리케이션된다
	ProjectileMovementComp->SetActive(HasAuthority());
}

void AMVE_ThrowObject::FireInDirection(const FVector& ShootDirection)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this]()
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	});
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
	ProjectileMovementComp->Velocity = ShootDirection * ProjectileMovementComp->InitialSpeed;
}