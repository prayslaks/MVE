// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_AudObject.generated.h"

class UBoxComponent;

UCLASS()
class MVE_API AMVE_StageLevel_AudObject : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_AudObject();
	
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION()
	void SetIsVisible(const bool Value);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MVE|Components")
	TObjectPtr<UBoxComponent> BoxComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MVE|Components")
	bool bIsVisible = true;
};