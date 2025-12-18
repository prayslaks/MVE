// Fill out your copyright notice in the Description page of Project Settings.


#include "StageLevel/Actor/Public/MVE_ThrowObject.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMVE_ThrowObject::AMVE_ThrowObject()
{
	//틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("Projectile"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

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
	
	// 서버만 물리 효과를 연산한다
	ProjectileMovementComp->SetActive(HasAuthority());
}

void AMVE_ThrowObject::FireInDirection(const FVector& ShootDirection)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this]()
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	});
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);
	ProjectileMovementComp->Velocity = ShootDirection * ProjectileMovementComp->InitialSpeed;
}
