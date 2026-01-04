#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVE_StageLevel_DummyAudienceManager.generated.h"

class AMVE_StageLevel_DummyAudience;

UCLASS()
class MVE_API AMVE_StageLevel_DummyAudienceManager : public AActor
{
	GENERATED_BODY()

public:
	AMVE_StageLevel_DummyAudienceManager();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPlayMusic();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnStopMusic();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AMVE_StageLevel_DummyAudience*> DummyAudiences;
};