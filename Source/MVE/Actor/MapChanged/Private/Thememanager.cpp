// ThemeManager.cpp

#include "ThemeManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

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

    DestroySpawnedActors();
    Super::EndPlay(EndPlayReason);
}

void AThemeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThemeManager, CurrentThemeIndex);
}

// ============================================================================
// STT 처리
// ============================================================================

void AThemeManager::OnSTTReceived(const FSTTResponse& Response)
{
    if (!HasAuthority()) return;
    if (!Response.bSuccess || Response.TranscribedText.IsEmpty()) return;

    const FString& Text = Response.TranscribedText;

    // 트리거 확인
    if (!HasTrigger(Text)) return;

    // 테마 찾기
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

// ============================================================================
// 테마 활성화
// ============================================================================

void AThemeManager::ActivateTheme(int32 ThemeIndex)
{
    if (!HasAuthority()) return;
    if (!Themes.IsValidIndex(ThemeIndex)) return;
    if (CurrentThemeIndex == ThemeIndex) return;

    CurrentThemeIndex = ThemeIndex;

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: 테마 %d 활성화"), ThemeIndex);

    Multicast_ActivateTheme(ThemeIndex);
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

void AThemeManager::ClearTheme()
{
    if (!HasAuthority()) return;

    CurrentThemeIndex = -1;
    Multicast_ActivateTheme(-1);
}

// ============================================================================
// RPC
// ============================================================================

void AThemeManager::Multicast_ActivateTheme_Implementation(int32 ThemeIndex)
{
    ApplyThemeLocally(ThemeIndex);
}

void AThemeManager::OnRep_CurrentTheme()
{
    ApplyThemeLocally(CurrentThemeIndex);
}

// ============================================================================
// 로컬 적용
// ============================================================================

void AThemeManager::ApplyThemeLocally(int32 ThemeIndex)
{
    // 기존 액터들 제거
    DestroySpawnedActors();

    if (!Themes.IsValidIndex(ThemeIndex))
    {
        OnThemeActivated.Broadcast(-1);
        return;
    }

    const FThemeEntry& Theme = Themes[ThemeIndex];

    // 새 액터들 스폰
    for (const FThemeActorSpawn& SpawnInfo : Theme.ActorsToSpawn)
    {
        if (!SpawnInfo.ActorClass) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Spawned = GetWorld()->SpawnActor<AActor>(
            SpawnInfo.ActorClass,
            SpawnInfo.SpawnTransform,
            Params
        );

        if (Spawned)
        {
            SpawnedActors.Add(Spawned);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ThemeManager: %d개 액터 스폰됨"), SpawnedActors.Num());

    OnThemeActivated.Broadcast(ThemeIndex);
}

void AThemeManager::DestroySpawnedActors()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();
}