// ThemeManager.h
// STT 음성 명령 → 테마 BP 액터 전환
// 오브젝트 풀링으로 즉각적인 테마 전환

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform SpawnTransform;

    // 풀링용: 태그로 같은 클래스 다른 인스턴스 구분 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActorTag;
};

// 테마 설정
USTRUCT(BlueprintType)
struct FThemeEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ThemeName;  // 디버깅/로깅용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Keywords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FThemeActorSpawn> ActorsToSpawn;
};

// 풀링된 액터 정보
USTRUCT()
struct FPooledThemeActors
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<TObjectPtr<AActor>> Actors;

    UPROPERTY()
    bool bIsActive = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThemeChanged, int32, OldThemeIndex, int32, NewThemeIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThemePoolReady, bool, bSuccess);

UCLASS(BlueprintType)
class MVE_API AThemeManager : public AActor
{
    GENERATED_BODY()

public:
    AThemeManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Config")
    TArray<FThemeEntry> Themes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Config")
    TArray<FString> TriggerKeywords = { TEXT("무대"), TEXT("바꿔"), TEXT("변경") };

    // 풀링 활성화 여부 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Pooling")
    bool bUseObjectPooling = true;

    // 자동으로 풀 초기화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Pooling")
    bool bAutoInitializePool = true;

    // 테마 전환 시 페이드 효과 (0이면 즉시 전환)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Transition", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TransitionDuration = 0.3f;

    // ===== 이벤트 =====

    UPROPERTY(BlueprintAssignable, Category = "Theme|Events")
    FOnThemeChanged OnThemeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Theme|Events")
    FOnThemePoolReady OnThemePoolReady;

    // ===== 상태 =====

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTheme, Category = "Theme|Status")
    int32 CurrentThemeIndex = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Theme|Status")
    bool bPoolInitialized = false;

    // ===== API =====

    // 풀 수동 초기화 (bAutoInitializePool = false일 때 사용)
    UFUNCTION(BlueprintCallable, Category = "Theme")
    void InitializePool();

    // 인덱스로 테마 활성화 (서버만)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    void ActivateTheme(int32 ThemeIndex);

    // 키워드로 테마 활성화
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    bool ActivateThemeByKeyword(const FString& Keyword);

    // 현재 테마 제거
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Theme")
    void ClearTheme();

    // 현재 활성 테마 이름 가져오기
    UFUNCTION(BlueprintPure, Category = "Theme")
    FName GetCurrentThemeName() const;

    // 특정 테마의 액터들 가져오기 (BP에서 추가 커스터마이징용)
    UFUNCTION(BlueprintPure, Category = "Theme")
    TArray<AActor*> GetThemeActors(int32 ThemeIndex) const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // RPC - Reliable 유지하되 중복 호출 방지 로직 추가
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ActivateTheme(int32 NewThemeIndex, int32 OldThemeIndex);

    UFUNCTION()
    void OnRep_CurrentTheme();

private:
    // ===== 풀링 데이터 =====

    // 테마 인덱스 → 풀링된 액터들
    UPROPERTY()
    TMap<int32, FPooledThemeActors> ThemeActorPool;

    // 중복 적용 방지용
    UPROPERTY()
    int32 LastAppliedThemeIndex = -1;

    // 이전 테마 인덱스 (OnRep용)
    UPROPERTY()
    int32 PreviousThemeIndex = -1;

    // 풀링 미사용 시 스폰된 액터들 
    UPROPERTY()
    TArray<TObjectPtr<AActor>> NonPooledSpawnedActors;
    

    void OnSTTReceived(const FSTTResponse& Response);
    int32 FindThemeByKeyword(const FString& Text) const;
    bool HasTrigger(const FString& Text) const;

    // 오브젝트 폴링 방식
    void SpawnPooledActorsForTheme(int32 ThemeIndex);
    void SetThemeActorsVisibility(int32 ThemeIndex, bool bVisible);
    void ApplyThemeFromPool(int32 NewThemeIndex, int32 OldThemeIndex);
    void ApplyThemeNonPooled(int32 ThemeIndex);
    void DestroyNonPooledActors();
    
    void CleanupPool();
};