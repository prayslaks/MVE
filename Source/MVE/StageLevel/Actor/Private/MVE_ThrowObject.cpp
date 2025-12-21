// Fill out your copyright notice in the Description page of Project Settings.


#include "StageLevel/Actor/Public/MVE_ThrowObject.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMVE_ThrowObject::AMVE_ThrowObject()
{
	//틱 활성화
	PrimaryActorTick.bCanEverTick = true;
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
	
	// 공기 저항
	DragCoefficient = 0.3f;

	// 발사체 컴포넌트
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->SetIsReplicated(true);
	ProjectileMovementComp->bComponentShouldUpdatePhysicsVolume = false;
	ProjectileMovementComp->SetInterpolatedComponent(MeshComp);
	ProjectileMovementComp->SetUpdatedComponent(RootComponent);
	ProjectileMovementComp->InitialSpeed = 3000.f;
	ProjectileMovementComp->MaxSpeed = 3000.f;
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->bShouldBounce = true;
	ProjectileMovementComp->Bounciness = 0.2f;
	ProjectileMovementComp->Friction = 0.3f;
	ProjectileMovementComp->ProjectileGravityScale = 0.5f;
	ProjectileMovementComp->SetActive(false);
}

void AMVE_ThrowObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && ProjectileMovementComp->IsActive())
	{
		// --- 양력 연산 ---
		if (IsValid(LiftCurve))
		{
			// 현재 속도 획득
			const FVector CurrentVelocity = ProjectileMovementComp->Velocity;

			// 커브 평가에 필요한 수평 속도 연산
			FVector HorizontalVelocityOnly = CurrentVelocity;
			HorizontalVelocityOnly.Z = 0.f;
			const float HorizontalSpeed = HorizontalVelocityOnly.Size();
			const float BaseLiftAcceleration = LiftCurve->GetFloatValue(HorizontalSpeed);

			// 수평 정렬도를 구해 양력 규모 조절에 활용 
			const FVector ForwardVector = GetActorForwardVector();
			const float HorizontalAlignment = 1.0f - FMath::Abs(FVector::DotProduct(ForwardVector, FVector::UpVector));
			
			// 기본 양력 가속도와 수평 정렬도를 곱해 최종값 연산
			const float FinalLiftAcceleration = BaseLiftAcceleration * HorizontalAlignment;
			
			// 수평 가속을 가해 양력을 시뮬레이션
			ProjectileMovementComp->Velocity.Z += FinalLiftAcceleration * DeltaTime;
		}
		
		// --- 공기 저항 ---
		if (DragCoefficient > 0.f)
		{
			// 현재 속도에 따른 공기 저항을 구해 가한다
			FVector& CurrentVelocity = ProjectileMovementComp->Velocity;
			CurrentVelocity -= CurrentVelocity * DragCoefficient * DeltaTime;
		}
	}
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