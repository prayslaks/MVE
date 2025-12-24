// ThemeSlotActor.h
// 맵에 배치 → 태그별로 메시가 바뀌는 액터
// STT "무대환경 크리스마스" → Theme.Christmas 태그 → 크리스마스 메시로 스왑

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"
#include "ThemeSlotActor.generated.h"

class USTTSubsystem;

// 태그별 메시 정의
USTRUCT(BlueprintType)
struct FThemeMeshEntry
{
	GENERATED_BODY()

	// 테마 태그 (예: Theme.Christmas)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ThemeTag;

	// 이 태그일 때 보여줄 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> Mesh;

	// 스케일 (옵션)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;
};

UCLASS(BlueprintType)
class MVE_API AThemeSlotActor : public AActor
{
	GENERATED_BODY()

public:
	AThemeSlotActor();

	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// 태그별 메시 목록 (에디터에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	TArray<FThemeMeshEntry> ThemeMeshes;

	// 기본 메시 (아무 태그도 매칭 안될 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	TSoftObjectPtr<UStaticMesh> DefaultMesh;

	// 현재 적용된 태그
	UPROPERTY(BlueprintReadOnly, Category = "Theme")
	FGameplayTag CurrentThemeTag;

	// 태그로 메시 변경
	UFUNCTION(BlueprintCallable, Category = "Theme")
	void ApplyTheme(FGameplayTag ThemeTag);

	// 기본 메시로 복귀
	UFUNCTION(BlueprintCallable, Category = "Theme")
	void ResetToDefault();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// STT 키워드 감지 콜백
	UFUNCTION()
	void OnKeywordDetected(FGameplayTag KeywordTag, const FString& FullText);

	// 메시 찾기
	const FThemeMeshEntry* FindMeshEntry(FGameplayTag Tag) const;
};