// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MVE_WS_ProjectilePool.generated.h"

struct FProjectilePool;
class AMVE_StageLevel_Projectile;

// USTRUCT for TMap value
USTRUCT()
struct FProjectilePool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AMVE_StageLevel_Projectile>> Pool;
};

/**
 * Projectile Object Pooling을 관리하는 월드 서브시스템입니다.
 */
UCLASS()
class MVE_API UMVE_WS_ProjectilePool : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 월드 비활성화 시 풀링된 오브젝트를 정리합니다.
	virtual void Deinitialize() override;

	/**
	 * 풀에서 발사체를 가져옵니다. 사용 가능한 발사체가 없으면 새로 생성합니다.
	 * @param Class 가져올 발사체의 클래스입니다.
	 * @return 사용 가능한 발사체 액터입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Projectile Pool")
	AMVE_StageLevel_Projectile* PopProjectile(TSubclassOf<AMVE_StageLevel_Projectile> Class);

	/**
	 * 사용이 끝난 발사체를 풀에 반환합니다.
	 * @param Projectile 반환할 발사체 액터입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Projectile Pool")
	void PushProjectile(AMVE_StageLevel_Projectile* Projectile);

private:
	/** 클래스별로 발사체를 저장하는 큐의 맵입니다. */
	UPROPERTY()
	TMap<UClass*, FProjectilePool> ProjectilePools;

	/**
	 * 요청된 클래스의 발사체를 스폰합니다.
	 * @param Class 스폰할 발사체의 클래스입니다.
	 * @return 스폰된 발사체 액터입니다.
	 */
	AMVE_StageLevel_Projectile* SpawnNewProjectile(TSubclassOf<AMVE_StageLevel_Projectile> Class) const;
};

