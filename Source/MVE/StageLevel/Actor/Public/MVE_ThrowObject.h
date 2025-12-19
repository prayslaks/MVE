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

	void FireInDirection(const FVector& ShootDirection);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.0"))
	float DragCoefficient = 0.05f; // 저항 값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UCurveFloat> LiftCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;
	
private:
	FTimerHandle TimerHandle;
	
};