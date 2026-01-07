
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_GS_StageLevel.generated.h"

class USenderReceiver;
struct FAssetMetadata;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewerCountChanged, int32, NewCount);

// 액세서리 다운로드 대기 중인 데이터
USTRUCT()
struct FPendingAccessoryData
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserID;

	UPROPERTY()
	FCustomizationData Data;
};

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

	// UserID로 던지기 메시 가져오기 (ThrowObject에서 사용)
	UFUNCTION(BlueprintCallable, Category = "MVE|Customization")
	UStaticMesh* GetThrowMeshForUser(const FString& UserID) const;

	// UserID로 던지기 메시 Scale 가져오기
	UFUNCTION(BlueprintCallable, Category = "MVE|Customization")
	float GetThrowMeshScaleForUser(const FString& UserID) const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ViewerCount)
	int32 ViewerCount;
	
	UFUNCTION()
	void OnRep_ViewerCount();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	 * 액세서리 네트워크 동기화
	 */
public:
	// 모든 클라이언트에게 액세서리 정보 브로드캐스트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_BroadcastAccessory(const FString& UserID, const FCustomizationData& CustomizationData);

	// UserID로 캐릭터 찾기
	APawn* FindCharacterByUserID(const FString& UserID) const;

protected:
	virtual void BeginPlay() override;

	

private:
	// 액세서리 다운로드 진행 중인 세션
	// Key: AssetID (FGuid), Value: {UserID, CustomizationData}
	UPROPERTY()
	TMap<FGuid, FPendingAccessoryData> PendingAccessories;

	// UserID별 던지기 메시 저장
	// Key: UserID, Value: ThrowMesh
	UPROPERTY()
	TMap<FString, UStaticMesh*> UserThrowMeshes;

	// UserID별 던지기 메시 Scale 저장
	// Key: UserID, Value: Scale
	UPROPERTY()
	TMap<FString, float> UserThrowMeshScales;

	// SenderReceiver 델리게이트 콜백
	UFUNCTION()
	void OnAccessoryLoaded(UObject* Asset, const FAssetMetadata& Metadata);

	// 액세서리를 캐릭터에 적용
	void ApplyAccessoryToCharacter(const FString& UserID, UObject* Asset, const FCustomizationData& Data);
};
