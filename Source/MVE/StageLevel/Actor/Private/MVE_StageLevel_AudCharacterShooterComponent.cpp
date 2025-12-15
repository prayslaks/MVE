#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacterShooterComponent.h"

#include "MVE.h"
#include "MVE_WS_ProjectilePool.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_Projectile.h"

class UMVE_WS_ProjectilePool;

UMVE_StageLevel_AudCharacterShooterComponent::UMVE_StageLevel_AudCharacterShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMVE_StageLevel_AudCharacterShooterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMVE_StageLevel_AudCharacterShooterComponent::Fire(const FVector& Location, const FVector& Direction)
{
	// 클라이언트는 서버에게 요청하고, 서버는 모든 클라이언트에게 멀티캐스트합니다.
	if (GetOwner()->HasAuthority())
	{
		Multicast_Fire(Location, Direction);
	}
	else
	{
		Server_Fire(Location, Direction);
	}
}

void UMVE_StageLevel_AudCharacterShooterComponent::Server_Fire_Implementation(const FVector& Location, const FVector& Direction)
{
	// 서버에서 수신한 요청을 모든 클라이언트에게 다시 전파합니다.
	Multicast_Fire(Location, Direction);
}

void UMVE_StageLevel_AudCharacterShooterComponent::Multicast_Fire_Implementation(const FVector& Location, const FVector& Direction)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 사운드 재생
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, FireSound, Location);
	}

	// 프로젝타일 풀 서브시스템 가져오기
	UMVE_WS_ProjectilePool* PoolSubsystem = World->GetSubsystem<UMVE_WS_ProjectilePool>();
	if (PoolSubsystem == nullptr)
	{
		PRINTLOG(TEXT("ShooterComponent: Projectile Pool Subsystem not found."));
		return;
	}

	// 프로젝타일 클래스가 설정되었는지 확인
	if (ProjectileClass == nullptr)
	{
		PRINTLOG(TEXT("ShooterComponent: ProjectileClass is not set."));
		return;
	}

	// 풀에서 프로젝타일 가져오기
	AMVE_StageLevel_Projectile* Projectile = PoolSubsystem->PopProjectile(ProjectileClass);
	if (Projectile == nullptr)
	{
		PRINTLOG(TEXT("ShooterComponent: Failed to pop projectile from pool."));
		return;
	}
	
	// 레이턴시 보상 계산
	FVector SpawnLocation = Location;
	if (const APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (const APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			// Ping은 왕복 시간이므로 단방향 레이턴시는 절반으로 연산
			const float Latency = PS->GetPingInMilliseconds() * 0.0005f;
			
			// 발사체의 보정된 위치 연산
			const float ProjectileSpeed = Projectile->GetProjectileSpeed();
			SpawnLocation += Direction * ProjectileSpeed * Latency;
		}
	}

	const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);
	Projectile->ActivateProjectile(SpawnTransform, Direction);
	PRINTLOG(TEXT("발사 컴포넌트 : %s에서 발사!"), *SpawnLocation.ToString());
}