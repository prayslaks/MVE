
#pragma once

#include "CoreMinimal.h"
#include "MVE_AUD_CustomizationManager.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_GM_StageLevel.generated.h"

// RPC용 액세서리 정보 구조체
USTRUCT(BlueprintType)
struct FPlayerAccessoryInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserID;

	UPROPERTY()
	FString PresetJSON;

	FPlayerAccessoryInfo()
		: UserID(TEXT(""))
		, PresetJSON(TEXT(""))
	{}

	FPlayerAccessoryInfo(const FString& InUserID, const FString& InPresetJSON)
		: UserID(InUserID)
		, PresetJSON(InPresetJSON)
	{}
};


class AMVE_StageLevel_ChatManager;

UCLASS()
class MVE_API AMVE_GM_StageLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMVE_GM_StageLevel();
	
	virtual void BeginPlay() override;

	// 호스트용 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MVE|Character")
	TSubclassOf<APawn> HostCharacterClass;

	// 클라이언트용 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MVE|Character")
	TSubclassOf<APawn> ClientCharacterClass;
	
	/**
	 * 모든 클라이언트에게 Presigned URL을 전송하여 오디오를 준비시킵니다.
	 * @param PresignedUrl 준비할 오디오의 Presigned URL입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Audio")
	void SendPresignedUrlToAllClients(const FString& PresignedUrl);

	/**
	 * 모든 클라이언트에게 준비된 오디오 재생을 요청합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Audio")
	void SendPlayCommandToAllClients();


	/**
	 * 모든 클라이언트에게 오디오 재생 중지를 요청합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Audio")
	void SendStopCommandToAllClients();

	/**
	 * 카메라 플래시 효과를 처리합니다.
	 * @param InstigatorController 플래시를 터뜨린 플레이어의 컨트롤러
	 * @param IgnoreActors
	 * @param FlashLocation 플래시 위치
	 * @param FlashDirection 플래시 방향
	 * @param EffectiveDistance 유효 거리
	 * @param FlashAngleDotThreshold 플래시 유효 각도의 Dot Product 값
	 */
	void HandleFlashEffect(const AController* InstigatorController, const TArray<AActor*>& IgnoreActors, const FVector& FlashLocation, const FVector& FlashDirection, float EffectiveDistance, float FlashAngleDotThreshold) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Flash Effect")
	float PlayerLookAtFlashDotThreshold = 0.8f;
	
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	// 호스트/클라이언트별 다른 플레이어 스타트를 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 호스트/클라이언트용 기본 폰 클래스
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	// 호스트인지 확인하는 헬퍼 함수
	bool IsHostController(AController* Controller) const;

private:
	// ChatManager 스폰
	void SpawnChatManager();

	// Host Controller
	UPROPERTY()
	TObjectPtr<APlayerController> HostController;
	
	// ChatManager
	UPROPERTY()
	TObjectPtr<AMVE_StageLevel_ChatManager> ChatManager;

	// 시청자 수 업데이트 타이머
	FTimerHandle ViewerCountUpdateTimerHandle;

	// 10초마다 시청자 수를 계산하고 GameState에 업데이트
	void UpdateViewerCount();


	/*
	 * 액세서리 네트워크 동기화
	 */
	
public:
	// PlayerController로부터 액세서리 정보 등록 받기
	void RegisterPlayerAccessory(const FString& UserID, const FString& PresetJSON);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
private:
	// 모든 클라이언트에게 액세서리 정보 브로드캐스트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_BroadcastAccessory(const FString& UserID, const FString& PresetJSON);
	
	// 현재 참여 중인 플레이어들의 액세서리 정보 저장
	// Key: UserID, Value: PresetJSON
	UPROPERTY()
	TMap<FString, FString> PlayerAccessories;

	// 액세서리 다운로드 진행 중인 세션
	// Key: UserID, Value: CustomizationData
	UPROPERTY()
	TMap<FString, FCustomizationData> PendingAccessories;

	// SenderReceiver 델리게이트 콜백
	UFUNCTION()
	void OnAccessoryLoaded(UObject* Asset, const FAssetMetadata& Metadata);
};
