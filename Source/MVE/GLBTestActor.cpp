
#include "GLBTestActor.h"

#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "Misc/Paths.h"

AGLBTestActor::AGLBTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // 스켈레탈 메시 컴포넌트 생성
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMeshComponent;
    
    // 기본 경로 설정 (Content/Characters/CesiumMan.glb)
    GLBFilePath = TEXT("Assets/Test/CesiumMan.glb");
}

void AGLBTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 자동으로 GLB 로드
    LoadGLBFile();
}

void AGLBTestActor::LoadGLBFile()
{
    // Content 폴더의 절대 경로 구하기
    FString ContentDir = FPaths::ProjectContentDir();
    FString FullPath = FPaths::Combine(ContentDir, GLBFilePath);
    
    UE_LOG(LogTemp, Log, TEXT("Attempting to load GLB from: %s"), *FullPath);
    
    // 파일 존재 확인
    if (!FPaths::FileExists(FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("GLB file not found: %s"), *FullPath);
        return;
    }
    
    // glTF Asset 로드
    UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
        FullPath,
        false,  // bPathRelativeToContent
        FglTFRuntimeConfig()
    );
    
    if (!Asset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load glTF Asset"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("✓ glTF Asset loaded successfully!"));
    
    // SkeletalMesh 생성 설정
    FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
    SkeletalMeshConfig.bOverwriteRefSkeleton = true;
    SkeletalMeshConfig.MorphTargetsDuplicateStrategy = EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;
    
    // SkeletalMesh 로드 (인덱스 0 = 첫 번째 메시)
    USkeletalMesh* LoadedMesh = Asset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);
    
    if (LoadedMesh)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ SkeletalMesh loaded successfully!"));
        OnSkeletalMeshLoaded(LoadedMesh);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load SkeletalMesh"));
    }
}

void AGLBTestActor::OnSkeletalMeshLoaded(USkeletalMesh* LoadedMesh)
{
    // 스켈레탈 메시 적용
    SkeletalMeshComponent->SetSkeletalMesh(LoadedMesh);
    
    // 본 구조 로그 출력
    const FReferenceSkeleton& RefSkeleton = LoadedMesh->GetRefSkeleton();
    int32 BoneCount = RefSkeleton.GetNum();
    
    UE_LOG(LogTemp, Log, TEXT("✓ Character applied! Bone count: %d"), BoneCount);
    
    // 본 이름 출력
    for (int32 i = 0; i < BoneCount; i++)
    {
        FName BoneName = RefSkeleton.GetBoneName(i);
        UE_LOG(LogTemp, Log, TEXT("  Bone[%d]: %s"), i, *BoneName.ToString());
    }
    
    // Morph Targets 확인
    if (LoadedMesh->GetMorphTargets().Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Morph Targets found: %d"), LoadedMesh->GetMorphTargets().Num());
        
        for (UMorphTarget* MorphTarget : LoadedMesh->GetMorphTargets())
        {
            UE_LOG(LogTemp, Log, TEXT("  MorphTarget: %s"), *MorphTarget->GetName());
        }
    }
}
