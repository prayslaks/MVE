#include "StageLevel/Actor/Public/MVE_StageLevel_Projectile.h"

#include "MVE_WS_ProjectilePool.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

class UMVE_WS_ProjectilePool;

AMVE_StageLevel_Projectile::AMVE_StageLevel_Projectile()
{
 	// 이 액터는 매 프레임 Tick()을 호출할 필요가 없으므로 성능을 위해 비활성화합니다.
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 리플리케이션을 비활성화합니다. 각 클라이언트가 독립적으로 시뮬레이션합니다.
	bReplicates = false;

	// 스태틱 메시 컴포넌트를 생성하고 루트 컴포넌트로 설정합니다.
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	
	// 기본 외형으로 구체를 사용하고 크기를 조절합니다.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	}
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	// 프로젝타일 무브먼트 컴포넌트를 생성합니다.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(MeshComponent);
	ProjectileMovementComponent->InitialSpeed = 3000.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.5f;

	// 생성자에서는 비활성화된 상태로 시작합니다.
    AActor::SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	ProjectileMovementComponent->Deactivate();
}

void AMVE_StageLevel_Projectile::ActivateProjectile(const FTransform& SpawnTransform, const FVector& ShootDirection)
{
	// 위치와 회전 설정
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// 상태 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 발사체 이동 활성화
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
	ProjectileMovementComponent->Activate();

	// 수명이 다하면 비활성화되도록 타이머 설정
	if (bIsPoolable)
	{
		// 유효한 타이머가 있다면 초기화
		if (PoolingTimerHandle.IsValid())
		{
			GetWorldTimerManager().ClearTimer(PoolingTimerHandle);
		}
		GetWorldTimerManager().SetTimer(PoolingTimerHandle, this, &AMVE_StageLevel_Projectile::DeactivateProjectile, LifeTime, false);
	}
	else
	{
		// 풀링을 사용하지 않으면 LifeTime 후 액터 파괴
		SetLifeSpan(LifeTime);
	}
}

void AMVE_StageLevel_Projectile::DeactivateProjectile()
{
	if (bIsPoolable)
	{
		// 타이머 핸들 비활성화
		if (PoolingTimerHandle.IsValid())
		{
			GetWorldTimerManager().ClearTimer(PoolingTimerHandle);
		}

		// 상태 비활성화
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		
		// 발사체 이동 정지 및 비활성화
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		ProjectileMovementComponent->Deactivate();

		// 풀에 반환
		if (const UWorld* World = GetWorld())
		{
			if (UMVE_WS_ProjectilePool* PoolSubsystem = World->GetSubsystem<UMVE_WS_ProjectilePool>())
			{
				PoolSubsystem->PushProjectile(this);
			}
		}
	}
	else
	{
		// 풀링을 사용하지 않는 경우 호출되지 않는 부분
	}
}