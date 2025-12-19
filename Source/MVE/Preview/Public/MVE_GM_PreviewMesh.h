#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MVE_GM_PreviewMesh.generated.h"

/**
 * PreviewMesh 전용 레벨 GameMode
 * 캐릭터 커스터마이징 프리뷰를 위한 GameMode
 */
UCLASS()
class MVE_API AMVE_GM_PreviewMesh : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMVE_GM_PreviewMesh();

	virtual void BeginPlay() override;

	// 프리뷰용 캐릭터 스폰 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	FVector CharacterSpawnLocation = FVector(0.0f, 0.0f, 0.0f);

	// 프리뷰용 캐릭터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	TSubclassOf<AActor> PreviewCharacterClass;

	// 스폰된 프리뷰 캐릭터 참조
	UFUNCTION(BlueprintCallable, Category = "Preview")
	AActor* GetPreviewCharacter() const { return PreviewCharacter; }

protected:
	virtual void StartPlay() override;

private:
	UPROPERTY()
	AActor* PreviewCharacter;

	// 프리뷰 캐릭터 스폰
	void SpawnPreviewCharacter();
};
