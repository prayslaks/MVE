// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_WS_ProjectilePool.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_Projectile.h"
#include "MVE.h"

void UMVE_WS_ProjectilePool::Deinitialize()
{
	// 모든 풀을 순회하며 유효한 모든 발사체 액터를 파괴합니다.
	for (auto& Pair : ProjectilePools)
	{
		for (AMVE_StageLevel_Projectile* Projectile : Pair.Value.Pool)
		{
			if (Projectile && IsValid(Projectile))
			{
				Projectile->Destroy();
			}
		}
	}
	ProjectilePools.Empty();

	Super::Deinitialize();
}

AMVE_StageLevel_Projectile* UMVE_WS_ProjectilePool::PopProjectile(TSubclassOf<AMVE_StageLevel_Projectile> Class)
{
	if (!Class)
	{
		return nullptr;
	}

	FProjectilePool& Pool = ProjectilePools.FindOrAdd(Class);
	AMVE_StageLevel_Projectile* Projectile = nullptr;

	if (Pool.Pool.Num() > 0)
	{
		Projectile = Pool.Pool.Pop();
		// 혹시나 배열에 유효하지 않은 포인터가 남아있을 경우를 대비
		if (!IsValid(Projectile))
		{
			// 유효하지 않으면 재귀 호출로 다음 것을 뽑거나 새로 생성
			PRINTLOG(TEXT("Projectile pool contained an invalid object, getting next one."));
			return PopProjectile(Class);
		}
		PRINTLOG(TEXT("Popped projectile from pool. Pool size is now: %d"), Pool.Pool.Num());
	}
	else
	{
		Projectile = SpawnNewProjectile(Class);
		if (Projectile)
		{
			PRINTLOG(TEXT("Projectile pool was empty. Spawned a new projectile."));
		}
	}

	return Projectile;
}

void UMVE_WS_ProjectilePool::PushProjectile(AMVE_StageLevel_Projectile* Projectile)
{
	if (!Projectile)
	{
		return;
	}

	UClass* Class = Projectile->GetClass();
	FProjectilePool& Pool = ProjectilePools.FindOrAdd(Class);
	Pool.Pool.Add(Projectile);
	PRINTLOG(TEXT("Pushed projectile to pool. Pool size for class '%s' is now: %d"), *Class->GetName(), Pool.Pool.Num());
}

AMVE_StageLevel_Projectile* UMVE_WS_ProjectilePool::SpawnNewProjectile(TSubclassOf<AMVE_StageLevel_Projectile> Class) const
{
	UWorld* World = GetWorld();
	if (!World || !Class)
	{
		return nullptr;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<AMVE_StageLevel_Projectile>(Class, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}
