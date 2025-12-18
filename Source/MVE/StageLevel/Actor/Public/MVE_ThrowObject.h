// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_ThrowObject.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class MVE_API AMVE_ThrowObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AMVE_ThrowObject();
	
	virtual void BeginPlay() override;

	void FireInDirection(const FVector& ShootDirection);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;
	
private:
	FTimerHandle TimerHandle;
	
};