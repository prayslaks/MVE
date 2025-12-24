// ThemeSlotActor.cpp

#include "../Public/ThemeSlotActor.h"
#include "STT/Public/STTSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

AThemeSlotActor::AThemeSlotActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    SetRootComponent(MeshComp);
    MeshComp->SetMobility(EComponentMobility::Movable);
}

void AThemeSlotActor::BeginPlay()
{
    Super::BeginPlay();

    // 기본 메시 적용
    if (!DefaultMesh.IsNull())
    {
        if (UStaticMesh* M = DefaultMesh.LoadSynchronous())
        {
            MeshComp->SetStaticMesh(M);
        }
    }

    // STT 서브시스템에 바인딩
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USTTSubsystem* STT = GI->GetSubsystem<USTTSubsystem>())
        {
            STT->OnKeywordDetected.AddDynamic(this, &AThemeSlotActor::OnKeywordDetected);
        }
    }
}

void AThemeSlotActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 바인딩 해제
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USTTSubsystem* STT = GI->GetSubsystem<USTTSubsystem>())
        {
            STT->OnKeywordDetected.RemoveDynamic(this, &AThemeSlotActor::OnKeywordDetected);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void AThemeSlotActor::OnKeywordDetected(FGameplayTag KeywordTag, const FString& FullText)
{
    // "무대환경" 키워드가 포함되어 있고, 테마 태그가 매칭되면 적용
    // 또는 직접 테마 태그가 들어오면 적용
    
    // ThemeMeshes에 해당 태그가 있는지 확인
    if (FindMeshEntry(KeywordTag))
    {
        ApplyTheme(KeywordTag);
    }
}

void AThemeSlotActor::ApplyTheme(FGameplayTag ThemeTag)
{
    const FThemeMeshEntry* Entry = FindMeshEntry(ThemeTag);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("ThemeSlotActor: 태그 '%s'에 해당하는 메시 없음"), *ThemeTag.ToString());
        return;
    }

    CurrentThemeTag = ThemeTag;

    // 메시 로드 및 적용
    if (!Entry->Mesh.IsNull())
    {
        // 비동기 로드
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(
            Entry->Mesh.ToSoftObjectPath(),
            FStreamableDelegate::CreateLambda([this, Entry]()
            {
                if (IsValid(this) && Entry->Mesh.IsValid())
                {
                    MeshComp->SetStaticMesh(Entry->Mesh.Get());
                    MeshComp->SetRelativeScale3D(Entry->Scale);
                }
            })
        );
    }

    UE_LOG(LogTemp, Log, TEXT("ThemeSlotActor [%s]: 테마 '%s' 적용"), *GetName(), *ThemeTag.ToString());
}

void AThemeSlotActor::ResetToDefault()
{
    CurrentThemeTag = FGameplayTag::EmptyTag;

    if (!DefaultMesh.IsNull())
    {
        if (UStaticMesh* M = DefaultMesh.LoadSynchronous())
        {
            MeshComp->SetStaticMesh(M);
            MeshComp->SetRelativeScale3D(FVector::OneVector);
        }
    }
}

const FThemeMeshEntry* AThemeSlotActor::FindMeshEntry(FGameplayTag Tag) const
{
    for (const FThemeMeshEntry& Entry : ThemeMeshes)
    {
        if (Entry.ThemeTag.MatchesTagExact(Tag))
        {
            return &Entry;
        }
    }
    return nullptr;
}