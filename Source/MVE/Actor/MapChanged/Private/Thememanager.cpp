// ThemeManager.cpp

#include "ThemeManager.h"
#include "Net/UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Components/PrimitiveComponent.h"

AThemeManager::AThemeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AThemeManager::BeginPlay()
{
    Super::BeginPlay();

    // STT 바인딩
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USTTSubsystem* STT = GI->GetSubsystem<USTTSubsystem>())
        {
            STT->OnSTTTextStream.AddDynamic(this, &AThemeManager::OnSTTReceived);
        }
    }

    // 오브젝트 풀 초기화 (모든 머신에서)
    if (bUseObjectPooling && bAutoInitializePool)
    {
        InitializePool();
    }
    SetupDebugInput();
}

void AThemeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USTTSubsystem* STT = GI->GetSubsystem<USTTSubsystem>())
        {
            STT->OnSTTTextStream.RemoveDynamic(this, &AThemeManager::OnSTTReceived);
        }
    }

    CleanupPool();
    DestroyNonPooledActors();

    Super::EndPlay(EndPlayReason);
}

void AThemeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThemeManager, CurrentThemeIndex);
}


// 오브젝트 풀링 초기화


void AThemeManager::InitializePool()
{
    if (bPoolInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("ThemeManager: 풀이 이미 초기화됨"));
        return;
    }

    if (!bUseObjectPooling)
    {
        UE_LOG(LogTemp, Warning, TEXT("ThemeManager: 풀링 비활성화 상태"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        OnThemePoolReady.Broadcast(false);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: %d개 테마에 대해 오브젝트 풀 초기화 시작"), Themes.Num());

    int32 TotalActorsSpawned = 0;

    for (int32 ThemeIdx = 0; ThemeIdx < Themes.Num(); ++ThemeIdx)
    {
        SpawnPooledActorsForTheme(ThemeIdx);
        
        // 초기에는 모두 숨김
        SetThemeActorsVisibility(ThemeIdx, false);

        if (ThemeActorPool.Contains(ThemeIdx))
        {
            TotalActorsSpawned += ThemeActorPool[ThemeIdx].Actors.Num();
        }
    }

    bPoolInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: 풀 초기화 완료 - 총 %d개 액터 스폰됨"), TotalActorsSpawned);

    OnThemePoolReady.Broadcast(true);
}

void AThemeManager::SpawnPooledActorsForTheme(int32 ThemeIndex)
{
    if (!Themes.IsValidIndex(ThemeIndex)) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FThemeEntry& Theme = Themes[ThemeIndex];
    FPooledThemeActors& PoolEntry = ThemeActorPool.FindOrAdd(ThemeIndex);

    for (const FThemeActorSpawn& SpawnInfo : Theme.ActorsToSpawn)
    {
        if (!SpawnInfo.ActorClass) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* SpawnedActor = World->SpawnActor<AActor>(
            SpawnInfo.ActorClass,
            SpawnInfo.SpawnTransform,
            Params
        );

        if (SpawnedActor)
        {
            // 태그 설정 (나중에 식별용)
            if (!SpawnInfo.ActorTag.IsNone())
            {
                SpawnedActor->Tags.Add(SpawnInfo.ActorTag);
            }

            // 초기 상태: 숨김 + 콜리전 비활성화
            SpawnedActor->SetActorHiddenInGame(true);
            SpawnedActor->SetActorEnableCollision(false);
            SpawnedActor->SetActorTickEnabled(false);

            PoolEntry.Actors.Add(SpawnedActor);

            UE_LOG(LogTemp, Verbose, TEXT("ThemeManager: [테마 %d] %s 스폰 완료"),
                ThemeIndex, *SpawnedActor->GetName());
        }
    }

    PoolEntry.bIsActive = false;
}

void AThemeManager::SetThemeActorsVisibility(int32 ThemeIndex, bool bVisible)
{
    FPooledThemeActors* PoolEntry = ThemeActorPool.Find(ThemeIndex);
    if (!PoolEntry) return;

    for (AActor* Actor : PoolEntry->Actors)
    {
        if (!IsValid(Actor)) continue;

        Actor->SetActorHiddenInGame(!bVisible);
        Actor->SetActorEnableCollision(bVisible);
        Actor->SetActorTickEnabled(bVisible);

        // 커스텀 활성화 이벤트 (BP에서 오버라이드 가능하도록)
        if (bVisible)
        {
            
        }
    }

    PoolEntry->bIsActive = bVisible;
}


// STT 처리


void AThemeManager::OnSTTReceived(const FSTTResponse& Response)
{
    if (!HasAuthority()) return;
    if (!Response.bSuccess || Response.TranscribedText.IsEmpty()) return;

    const FString& Text = Response.TranscribedText;

    if (!HasTrigger(Text)) return;

    int32 FoundIndex = FindThemeByKeyword(Text);
    if (FoundIndex >= 0)
    {
        ActivateTheme(FoundIndex);
    }
}

bool AThemeManager::HasTrigger(const FString& Text) const
{
    for (const FString& Trigger : TriggerKeywords)
    {
        if (Text.Contains(Trigger, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

int32 AThemeManager::FindThemeByKeyword(const FString& Text) const
{
    for (int32 i = 0; i < Themes.Num(); ++i)
    {
        for (const FString& Keyword : Themes[i].Keywords)
        {
            if (Text.Contains(Keyword, ESearchCase::IgnoreCase))
            {
                return i;
            }
        }
    }
    return -1;
}


// 테마 활성화


void AThemeManager::ActivateTheme(int32 ThemeIndex)
{
    if (!HasAuthority()) return;
    if (!Themes.IsValidIndex(ThemeIndex) && ThemeIndex != -1) return;
    
    // 같은 테마면 무시
    if (CurrentThemeIndex == ThemeIndex) return;

    int32 OldThemeIndex = CurrentThemeIndex;
    PreviousThemeIndex = OldThemeIndex; 
    CurrentThemeIndex = ThemeIndex;

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: 테마 전환 %d → %d"), OldThemeIndex, ThemeIndex);

    // Multicast로 모든 클라이언트에 즉시 적용
    // OnRep은 나중에 오므로, Multicast에서만 실제 적용
    Multicast_ActivateTheme(ThemeIndex, OldThemeIndex);
}

bool AThemeManager::ActivateThemeByKeyword(const FString& Keyword)
{
    int32 Index = FindThemeByKeyword(Keyword);
    if (Index >= 0)
    {
        ActivateTheme(Index);
        return true;
    }
    return false;
}

FName AThemeManager::GetCurrentThemeName() const
{
    if (Themes.IsValidIndex(CurrentThemeIndex))
    {
        return Themes[CurrentThemeIndex].ThemeName;
    }
    return NAME_None;
}

TArray<AActor*> AThemeManager::GetThemeActors(int32 ThemeIndex) const
{
    TArray<AActor*> Result;
    
    if (bUseObjectPooling)
    {
        const FPooledThemeActors* PoolEntry = ThemeActorPool.Find(ThemeIndex);
        if (PoolEntry)
        {
            for (AActor* Actor : PoolEntry->Actors)
            {
                if (IsValid(Actor))
                {
                    Result.Add(Actor);
                }
            }
        }
    }
    else
    {
        // 비풀링 모드에서는 현재 테마만 반환 가능
        if (ThemeIndex == CurrentThemeIndex)
        {
            for (AActor* Actor : NonPooledSpawnedActors)
            {
                if (IsValid(Actor))
                {
                    Result.Add(Actor);
                }
            }
        }
    }

    return Result;
}


// RPC 구현

void AThemeManager::Multicast_ActivateTheme_Implementation(int32 NewThemeIndex, int32 OldThemeIndex)
{
    // 중복 적용 방지: 이미 이 테마가 적용됐으면 스킵
    if (LastAppliedThemeIndex == NewThemeIndex)
    {
        UE_LOG(LogTemp, Verbose, TEXT("ThemeManager: 테마 %d 이미 적용됨, 스킵"), NewThemeIndex);
        return;
    }

    LastAppliedThemeIndex = NewThemeIndex;

    if (bUseObjectPooling)
    {
        ApplyThemeFromPool(NewThemeIndex, OldThemeIndex);
    }
    else
    {
        ApplyThemeNonPooled(NewThemeIndex);
    }

    OnThemeChanged.Broadcast(OldThemeIndex, NewThemeIndex);
}

void AThemeManager::OnRep_CurrentTheme()
{
    // Multicast가 먼저 도착했을 수 있으므로 중복 체크
    if (LastAppliedThemeIndex == CurrentThemeIndex)
    {
        UE_LOG(LogTemp, Verbose, TEXT("ThemeManager: OnRep - 테마 %d 이미 적용됨"), CurrentThemeIndex);
        return;
    }

    // Multicast를 못 받은 경우에만 여기서 적용 (네트워크 순서 역전 대비)
    LastAppliedThemeIndex = CurrentThemeIndex;

    if (bUseObjectPooling)
    {
        ApplyThemeFromPool(CurrentThemeIndex, PreviousThemeIndex);
    }
    else
    {
        ApplyThemeNonPooled(CurrentThemeIndex);
    }

    OnThemeChanged.Broadcast(PreviousThemeIndex, CurrentThemeIndex);
}


// 풀링 방식 적용


void AThemeManager::ApplyThemeFromPool(int32 NewThemeIndex, int32 OldThemeIndex)
{
    // 풀 초기화 안됐으면 지금 초기화
    if (!bPoolInitialized)
    {
        InitializePool();
    }

    // 이전 테마 숨김
    if (OldThemeIndex >= 0)
    {
        SetThemeActorsVisibility(OldThemeIndex, false);
        UE_LOG(LogTemp, Log, TEXT("ThemeManager: 테마 %d 비활성화"), OldThemeIndex);
    }

    // 새 테마 표시
    if (NewThemeIndex >= 0)
    {
        SetThemeActorsVisibility(NewThemeIndex, true);
        UE_LOG(LogTemp, Log, TEXT("ThemeManager: 테마 %d 활성화 - %d개 액터"),
            NewThemeIndex,
            ThemeActorPool.Contains(NewThemeIndex) ? ThemeActorPool[NewThemeIndex].Actors.Num() : 0);
    }
}


// 비풀링 방식


void AThemeManager::ApplyThemeNonPooled(int32 ThemeIndex)
{
    // 기존 액터들 제거
    DestroyNonPooledActors();

    if (!Themes.IsValidIndex(ThemeIndex))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    const FThemeEntry& Theme = Themes[ThemeIndex];

    for (const FThemeActorSpawn& SpawnInfo : Theme.ActorsToSpawn)
    {
        if (!SpawnInfo.ActorClass) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Spawned = World->SpawnActor<AActor>(
            SpawnInfo.ActorClass,
            SpawnInfo.SpawnTransform,
            Params
        );

        if (Spawned)
        {
            NonPooledSpawnedActors.Add(Spawned);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ThemeManager (NonPooled): %d개 액터 스폰됨"), NonPooledSpawnedActors.Num());
}

void AThemeManager::DestroyNonPooledActors()
{
    for (AActor* Actor : NonPooledSpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    NonPooledSpawnedActors.Empty();
}


void AThemeManager::CleanupPool()
{
    for (auto& Pair : ThemeActorPool)
    {
        for (AActor* Actor : Pair.Value.Actors)
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
            }
        }
        Pair.Value.Actors.Empty();
    }
    ThemeActorPool.Empty();
    bPoolInitialized = false;
}

void AThemeManager::SetupDebugInput()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        // 이 액터가 입력을 받을 수 있도록 활성화한다.
        EnableInput(PC);

        if (InputComponent)
        {
            // 숫자 1, 2, 3 키를 각 테마에 바인딩한다.
            InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AThemeManager::OnDebugKey_Theme1);
            InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AThemeManager::OnDebugKey_Theme2);
            InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AThemeManager::OnDebugKey_Theme3);

            // 스페이스바를 누르면 다음 테마로 순환한다.
            InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AThemeManager::OnDebugKey_Next);
            
            UE_LOG(LogTemp, Log, TEXT("ThemeManager: 디버그 키 바인딩 완료 (1, 2, 3, Space)"));
        }
    }
}

bool AThemeManager::HasClearKeyword(const FString& Text) const
{
        for (const FString& Keyword : ClearKeywords)
        {
            if (Text.Contains(Keyword, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }
        return false;
}

void AThemeManager::OnDebugKey_Theme1()
{
    // 인덱스 0번 테마 활성화
    UE_LOG(LogTemp, Log, TEXT("ThemeManager: [Debug] 1번 키 입력 -> 테마 0 전환 시도"));
    ActivateTheme(0); 
}

void AThemeManager::OnDebugKey_Theme2()
{
    // 인덱스 1번 테마 활성화
    UE_LOG(LogTemp, Log, TEXT("ThemeManager: [Debug] 2번 키 입력 -> 테마 1 전환 시도"));
    ActivateTheme(1);
}

void AThemeManager::OnDebugKey_Theme3()
{
    // 인덱스 2번 테마 활성화
    UE_LOG(LogTemp, Log, TEXT("ThemeManager: [Debug] 3번 키 입력 -> 테마 2 전환 시도"));
    ActivateTheme(2);
}

void AThemeManager::OnDebugKey_Next()
{
    // 현재 테마 다음 번호로 순환 (토글 기능)
    int32 NextIndex = (CurrentThemeIndex + 1) % Themes.Num();
    
    // 테마가 하나도 없으면 중단
    if (Themes.Num() == 0) return;

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: [Debug] Space 입력 -> 다음 테마(%d) 전환"), NextIndex);
    ActivateTheme(NextIndex);
}