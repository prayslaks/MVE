#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_SpotlightManager.generated.h"

class AMVE_StageLevel_Spotlight;

USTRUCT(BlueprintType)
struct FSpotlightGroup
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AMVE_StageLevel_Spotlight*> Spotlights;
};

UCLASS()
class MVE_API AMVE_StageLevel_SpotlightManager : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_SpotlightManager();
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "MVE|SequenceActor")
	void ExecuteSequence(int32 SequenceNumber, float DelayBetweenOrder);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	TMap<int32, FSpotlightGroup> SpotlightsBySequenceOrder;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "MVE|SequenceActor")
	TArray<int32> SequenceOrders;
	
	FTimerHandle ExecutionDelayTimerHandle;
};