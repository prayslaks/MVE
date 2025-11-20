
#include "GLBTestActor.h"

#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "MVE.h"
#include "Misc/Paths.h"
#include "Rig/IKRigDefinition.h"

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
    LoadAndTestRetargeting();
}

USkeletalMesh* AGLBTestActor::LoadGLBFile()
{
    // Content 폴더의 절대 경로 구하기
    FString ContentDir = FPaths::ProjectContentDir();
    FString FullPath = FPaths::Combine(ContentDir, GLBFilePath);
    
    PRINTLOG(TEXT("Attempting to load GLB from: %s"), *FullPath);
    
    // 파일 존재 확인
    if (!FPaths::FileExists(FullPath))
    {
        PRINTLOG(TEXT("GLB file not found: %s"), *FullPath);
        return nullptr;
    }
    
    // glTF Asset 로드
    UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
        FullPath,
        false,  // bPathRelativeToContent
        FglTFRuntimeConfig()
    );
    
    if (!Asset)
    {
        PRINTLOG(TEXT("Failed to load glTF Asset"));
        return nullptr;
    }
    
    PRINTLOG(TEXT("✓ glTF Asset loaded successfully!"));

    
    // SkeletalMesh 생성 설정
    FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
    SkeletalMeshConfig.bOverwriteRefSkeleton = true;
    SkeletalMeshConfig.MorphTargetsDuplicateStrategy = EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;
    
    // SkeletalMesh 로드 (인덱스 0 = 첫 번째 메시)
    USkeletalMesh* LoadedMesh = Asset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);
    return LoadedMesh;

    /*
    if (LoadedMesh)
    {
        PRINTLOG(TEXT("✓ SkeletalMesh loaded successfully!"));
        OnSkeletalMeshLoaded(LoadedMesh);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load SkeletalMesh"));
    }
    */
}

void AGLBTestActor::LoadAndTestRetargeting()
{
    PRINTLOG(TEXT("=== Runtime Retargeting Test Start ==="));
    
    // 1. GLB 파일 로드
    USkeletalMesh* LoadedMesh = LoadGLBFile();
    
    if (!LoadedMesh)
    {
        PRINTLOG(TEXT("Failed to load GLB file: %s"), *GLBFilePath);
        return;
    }
    
    PRINTLOG(TEXT("GLB file loaded successfully!"));
    
    // 2. 로드된 메시 적용
    SkeletalMeshComponent->SetSkeletalMesh(LoadedMesh);
    
    // 3. 본 구조 출력 (디버깅)
    PrintBoneStructure(LoadedMesh);
    
    // 4. IK Rig 확인
    if (StandardIKRig)
    {
        PRINTLOG(TEXT("Standard IK Rig is set: %s"), 
               *StandardIKRig->GetName());
        
        // 다음 단계에서 실제 리타게팅 로직 추가 예정
    }
    else
    {
        PRINTLOG(TEXT("Standard IK Rig is not set! Please assign it in the editor."));
    }
    
    PRINTLOG(TEXT("=== Test Complete ==="));
}

void AGLBTestActor::OnSkeletalMeshLoaded(USkeletalMesh* LoadedMesh)
{
    // 스켈레탈 메시 적용
    SkeletalMeshComponent->SetSkeletalMesh(LoadedMesh);
    
    // 본 구조 로그 출력
    const FReferenceSkeleton& RefSkeleton = LoadedMesh->GetRefSkeleton();
    int32 BoneCount = RefSkeleton.GetNum();
    
    PRINTLOG(TEXT("✓ Character applied! Bone count: %d"), BoneCount);
    
    // 본 이름 출력
    for (int32 i = 0; i < BoneCount; i++)
    {
        FName BoneName = RefSkeleton.GetBoneName(i);
        PRINTLOG(TEXT("  Bone[%d]: %s"), i, *BoneName.ToString());
    }
    
    // Morph Targets 확인
    if (LoadedMesh->GetMorphTargets().Num() > 0)
    {
        PRINTLOG(TEXT("✓ Morph Targets found: %d"), LoadedMesh->GetMorphTargets().Num());
        
        for (UMorphTarget* MorphTarget : LoadedMesh->GetMorphTargets())
        {
            PRINTLOG(TEXT("  MorphTarget: %s"), *MorphTarget->GetName());
        }
    }
}

void AGLBTestActor::PrintBoneStructure(USkeletalMesh* Mesh)
{
    if (!Mesh)
        return;
    
    const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
    
    PRINTLOG(TEXT("=== Bone Structure ==="));
    PRINTLOG( TEXT("Total Bones: %d"), RefSkeleton.GetNum());
    
    for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
    {
        FName BoneName = RefSkeleton.GetBoneName(i);
        int32 ParentIndex = RefSkeleton.GetParentIndex(i);
        
        PRINTLOG(TEXT("[%d] %s (Parent: %d)"), 
               i, *BoneName.ToString(), ParentIndex);
    }
}
