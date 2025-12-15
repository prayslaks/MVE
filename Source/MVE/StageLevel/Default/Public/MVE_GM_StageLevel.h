
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_GM_StageLevel.generated.h"

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

protected:
	
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	// 호스트/클라이언트별 다른 플레이어 스타트를 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 호스트/클라이언트용 기본 폰 클래스
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	// 호스트인지 확인하는 헬퍼 함수
	bool IsHostController(AController* Controller) const;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> HostController;
};
