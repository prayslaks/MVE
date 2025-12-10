
#include "GLBTestActor.h"

#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "MVE.h"
#include "Misc/Paths.h"
#include "Retargeter/IKRetargetProcessor.h"
#include "Rig/IKRigDefinition.h"

AGLBTestActor::AGLBTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // 스켈레탈 메시 컴포넌트 생성
    SourceMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SourceMeshComponent;
    
    // 기본 경로 설정 (Content/Characters/CesiumMan.glb)
    GLBFilePath = TEXT("Assets/Test/3d_character_boy_with_rigging.glb");

    // 타겟 메시 컴포넌트 (리타게팅된 Mannequin)
    TargetMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TargetMesh"));
    TargetMeshComponent->SetupAttachment(RootComponent);
    TargetMeshComponent->SetRelativeLocation(FVector(0, 200, 0)); // 옆에 배치
    
    
}

void AGLBTestActor::CopyPoseFromSourceToTarget()
{
    
}

void AGLBTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*
    // 리타게팅 활성화되어 있고 Processor가 준비되었으면
    if (bEnableRetargeting && RetargetProcessor)
    {
        // ✅ 매 프레임 소스의 포즈를 타겟으로 복사!
        CopyPoseFromSourceToTarget();
    }
    */
}

void AGLBTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 자동으로 GLB 로드
    LoadAndTestRetargeting();

    // ✅ 중요: TargetMeshComponent를 SourceMeshComponent의 자식으로 설정
    TargetMeshComponent->AttachToComponent(SourceMeshComponent, 
        FAttachmentTransformRules::KeepRelativeTransform);
    
    // ✅ AnimBP가 Parent Component를 제대로 인식하도록 강제 업데이트
    if (UAnimInstance* AnimInst = TargetMeshComponent->GetAnimInstance())
    {
        AnimInst->Montage_Stop(0.0f); // 리셋
        AnimInst->UpdateAnimation(0.0f, false); // 강제 업데이트
    }
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

    if (LoadedMesh)
    {
        // ✅ 메시가 GC되지 않도록 보호
        LoadedMesh->AddToRoot();
        PRINTLOG(TEXT("✅ SkeletalMesh loaded and protected from GC"));
    }
    
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
    SourceMeshComponent->SetSkeletalMesh(LoadedMesh);

    TargetMeshComponent->InitAnim(true);
    PRINTLOG(TEXT("✅ AnimInstance reinitialized"));
    
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

    // 5. IK Rig 및 Retargeter 확인
    if (!SourceIKRig)
    {
        PRINTLOG(TEXT("ERROR: Source IK Rig is not set!"));
        return;
    }
    
    if (!StandardIKRig)
    {
        PRINTLOG(TEXT("ERROR: Target IK Rig is not set!"));
        return;
    }
    
    if (!RetargeterAsset)
    {
        PRINTLOG(TEXT("ERROR: Retargeter Asset is not set!"));
        return;
    }
    
    PRINTLOG(TEXT("All IK assets are configured"));
    
    // 6. 실제 리타게팅 수행
    if (PerformRetargeting())
    {
        PRINTLOG(TEXT("Retargeting SUCCESS!"));
    }
    else
    {
        PRINTLOG(TEXT("Retargeting FAILED!"));
    }

    

    if (SourceAnim)
    {
        SourceMeshComponent->PlayAnimation(SourceAnim, true);  // 루프 재생
        PRINTLOG(TEXT("Playing animation on source mesh"));
    }

    // ✅ TargetMeshComponent의 AnimMode 설정
    TargetMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    
    // ✅ AnimBP 클래스 설정 (에디터에서 설정했다면 생략 가능)
    if (AnimBPClass)
    {
        TargetMeshComponent->SetAnimInstanceClass(AnimBPClass);
    }
    
    PRINTLOG(TEXT("=== Test Complete ==="));
}

bool AGLBTestActor::PerformRetargeting()
{
    PRINTLOG(TEXT("=== Performing IK Retargeting ==="));
    
    if (!RetargeterAsset)
    {
        PRINTLOG(TEXT("Retargeter asset is null!"));
        return false;
    }
    
    // IK Retargeter Processor 생성
    RetargetProcessor = NewObject<UIKRetargetProcessor>();
    
    if (!RetargetProcessor)
    {
        PRINTLOG(TEXT("Failed to create IKRetargetProcessor"));
        return false;
    }

    FRetargetProfile Profile;
    
    // 프로세서 초기화
    RetargetProcessor->Initialize(
        SourceMeshComponent->GetSkeletalMeshAsset(),
        TargetMeshComponent->GetSkeletalMeshAsset(),
        RetargeterAsset,
        Profile
    );
    
    PRINTLOG(TEXT("IK Retarget Processor initialized"));
    
    // TODO: 다음 단계에서 실제 포즈 복사 구현
    // 지금은 설정이 올바른지만 확인

    if (SourceAnim)
        TargetMeshComponent->PlayAnimation(SourceAnim, true);
    
    return true;
}

void AGLBTestActor::OnSkeletalMeshLoaded(USkeletalMesh* LoadedMesh)
{
    // 스켈레탈 메시 적용
    SourceMeshComponent->SetSkeletalMesh(LoadedMesh);
    
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
