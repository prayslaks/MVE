
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MVE_GS_StageLevel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewerCountChanged, int32, NewCount);

/**
 * StageLevel 전용 Game State
 */
UCLASS()
class MVE_API AMVE_GS_StageLevel : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMVE_GS_StageLevel();
	
	UPROPERTY(BlueprintAssignable, Category = "MVE|Concert")
	FOnViewerCountChanged OnViewerCountChanged;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MVE|Concert")
	int32 GetViewerCount() const { return ViewerCount; }
	
	void SetViewerCount(int32 NewCount);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ViewerCount)
	int32 ViewerCount;
	
	UFUNCTION()
	void OnRep_ViewerCount();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
