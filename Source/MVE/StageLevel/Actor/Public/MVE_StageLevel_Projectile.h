#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MVE_StageLevel_Projectile.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class MVE_API AMVE_StageLevel_Projectile : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_Projectile();

	/** 
	 * 발사체를 활성화하고 지정된 위치와 방향으로 발사
	 * @param SpawnTransform 발사될 위치와 회전값
	 * @param ShootDirection 발사될 방향 벡터
	 */
	void ActivateProjectile(const FTransform& SpawnTransform, const FVector& ShootDirection);

	// 발사체를 반환
	UFUNCTION()
	void DeactivateProjectile();

	// 발사체의 속도 반환
	UFUNCTION()
	FORCEINLINE float GetProjectileSpeed() const { return ProjectileMovementComponent->InitialSpeed; }
	
protected:
	// 발사체의 풀링 사용 여부
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Projectile")
	bool bIsPoolable = true;
	
	// 풀에 반환되기 까지의 발사체의 수명
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Projectile")
	float LifeTime = 10.0f;

	// 발사체의 외형을 표시하는 스태틱 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 발사체의 움직임을 관리하는 발사체 무브먼트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

private:
	// 풀링 타이머 핸들
	FTimerHandle PoolingTimerHandle;
};