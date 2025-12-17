#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_Speaker.generated.h"

class UStaticMeshComponent;
class UAudioComponent;

UCLASS()
class MVE_API AMVE_StageLevel_Speaker : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_Speaker();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	FORCEINLINE UAudioComponent* GetAudioComponent() const { return AudioComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;
};
