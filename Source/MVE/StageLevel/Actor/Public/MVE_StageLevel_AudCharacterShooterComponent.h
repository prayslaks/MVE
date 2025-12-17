#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_StageLevel_AudCharacterShooterComponent.generated.h"

class AMVE_StageLevel_Projectile;
class USoundBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_StageLevel_AudCharacterShooterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_StageLevel_AudCharacterShooterComponent();

	/**
	 * 발사체를 발사합니다.
	 * 서버에서는 즉시 모든 클라이언트에게 멀티캐스트하고, 클라이언트에서는 서버에게 RPC를 보냅니다.
	 * @param Location 발사 시작 위치입니다.
	 * @param Direction 발사 방향입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Shooter")
	void Fire(const FVector& Location, const FVector& Direction);

protected:
	virtual void BeginPlay() override;

	/** 발사할 프로젝타일 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Projectile")
	TSubclassOf<AMVE_StageLevel_Projectile> ProjectileClass;

	/** 발사 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Projectile")
	TObjectPtr<USoundBase> FireSound;

private:
	/** 서버에게 발사를 요청하는 RPC */
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector& Location, const FVector& Direction);

	/** 모든 클라이언트에게 발사 비주얼을 동기화하는 RPC */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector& Location, const FVector& Direction);
};
