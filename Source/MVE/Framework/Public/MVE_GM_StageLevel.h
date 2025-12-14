
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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	void LoadCharacterClasses();

protected:
	virtual void BeginPlay() override;

	// 호스트용 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSubclassOf<APawn> HostCharacterClass;

	// 클라이언트용 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSubclassOf<APawn> ClientCharacterClass;
	
	//플레이어 컨트롤러에 따라 다른 캐릭터 클래스 반환
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// 호스트/클라이언트별 다른 PlayerStart 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 호스트인지 확인하는 헬퍼 함수
	bool IsHostController(AController* Controller) const;


private:
	
	UPROPERTY()
	TObjectPtr<APlayerController> HostController;
};
