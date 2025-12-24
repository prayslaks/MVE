// ThemeManager.h
// STT 카테고리 입력 → 해당 BP 액터들 스폰
// BP 액터는 사용자가 직접 제작

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "STT/Public/STTSubsystem.h"
#include "ThemeManager.generated.h"

// 스폰할 액터 정보
USTRUCT(BlueprintType)
struct FThemeActorSpawn
{
    GENERATED_BODY()

    // 스폰할 BP 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    // 스폰 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform SpawnTransform;
};

// 테마(카테고리) 설정
USTRUCT(BlueprintType)
struct FThemeEntry
{
    GENERATED_BODY()

    // 인식 키워드들 (예: "크리스마스", "christmas", "xmas")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Keywords;

    // 이 테마에서 스폰할 액터들
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FThemeActorSpawn> ActorsToSpawn;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThemeActivated, int32, ThemeIndex);

UCLASS(BlueprintType)
class MVE_API AThemeManager : public AActor
{
    GENERATED_BODY()

public:
    AThemeManager();

    // ===== 설정 (에디터에서) =====

    // 테마 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
    TArray<FThemeEntry> Themes;

    // 무대환경 변경 트리거 (예: "무대", "바꿔")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
    TArray<FString> TriggerKeywords = { TEXT("무대"), TEXT("바꿔"), TEXT("변경") };

    // ===== 이벤트 =====

    UPROPERTY(BlueprintAssignable)
    FOnThemeActivated OnThemeActivated;

    // ===== 상태 =====

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTheme, Category = "Theme")
    int32 CurrentThemeIndex = -1;

    // ===== API =====

    // 인덱스로 테마 활성화
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    void ActivateTheme(int32 ThemeIndex);

    // 키워드로 테마 활성화
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    bool ActivateThemeByKeyword(const FString& Keyword);

    // 현재 테마 제거
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    void ClearTheme();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ActivateTheme(int32 ThemeIndex);

    UFUNCTION()
    void OnRep_CurrentTheme();

private:
    // 현재 스폰된 테마 액터들
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedActors;

    // STT 콜백
    UFUNCTION()
    void OnSTTReceived(const FSTTResponse& Response);

    // 키워드 → 테마 인덱스 찾기
    int32 FindThemeByKeyword(const FString& Text) const;

    // 트리거 확인
    bool HasTrigger(const FString& Text) const;

    // 로컬 테마 적용
    void ApplyThemeLocally(int32 ThemeIndex);

    // 스폰된 액터들 제거
    void DestroySpawnedActors();
};