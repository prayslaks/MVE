// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Curves/CurveFloat.h"
#include "MVE_ThrowObject.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class MVE_API AMVE_ThrowObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AMVE_ThrowObject();

	virtual void Tick(float DeltaTime) override;
	
	virtual void BeginPlay() override;

	// ⭐ 모든 머신에서 발사 (멀티캐스트 RPC)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireInDirection(const FVector& ShootDirection, float Speed);

	// 커스텀 메시 설정 (멀티캐스트 RPC)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCustomMesh(UStaticMesh* NewMesh);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.0"))
	float DragCoefficient = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UCurveFloat> LiftCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

public:
	// Owner의 UserID (던진 사람의 UserID)
	UPROPERTY(ReplicatedUsing = OnRep_OwnerUserID)
	FString OwnerUserID;

	UFUNCTION()
	void OnRep_OwnerUserID();

private:
	FTimerHandle TimerHandle;

};