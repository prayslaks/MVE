
#include "GLBRetargetTestActor.h"

#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "MVE.h"
#include "Retargeter/IKRetargetProcessor.h"
#include "Retargeter/IKRetargetProfile.h"

AGLBRetargetTestActor::AGLBRetargetTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 소스 컴포넌트 (Mannequin)
	SourceMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SourceMesh"));
	RootComponent = SourceMeshComponent;
    
	// 타겟 컴포넌트 (GLB 로드용)
	TargetMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TargetMesh"));
	TargetMeshComponent->SetupAttachment(RootComponent);
	TargetMeshComponent->SetRelativeLocation(FVector(0, 200, 0)); // 옆에 배치
}

void AGLBRetargetTestActor::BeginPlay()
{
	Super::BeginPlay();

	PRINTLOG(TEXT("=== GLB Runtime Retargeting System Starting ==="));
    
	// 1. 소스 메시 설정 (Mannequin)
	if (SourceSkeletalMesh)
	{
		SourceMeshComponent->SetSkeletalMesh(SourceSkeletalMesh);
		PRINTLOG(TEXT("✅ Source mesh (Mannequin) set"));
        
		// 소스 애니메이션 재생 (테스트용)
		if (SourceAnimation)
		{
			SourceMeshComponent->PlayAnimation(SourceAnimation, true);
			PRINTLOG(TEXT("✅ Source animation playing"));
		}
	}
	else
	{
		PRINTLOG(TEXT("⚠️ Source mesh not set! Please assign in editor"));
	}
    
	// 2. 동적 리타게팅 설정
	if (SetupDynamicRetargeting())
	{
		PRINTLOG(TEXT("✅ Dynamic retargeting setup complete!"));
        
		// 3. 타겟을 소스의 자식으로 설정 (AnimBP를 위해)
		TargetMeshComponent->AttachToComponent(SourceMeshComponent, 
			FAttachmentTransformRules::KeepRelativeTransform);
        
		// 4. AnimBP 업데이트
		if (UAnimInstance* AnimInst = TargetMeshComponent->GetAnimInstance())
		{
			AnimInst->UpdateAnimation(0.0f, false);
			PRINTLOG(TEXT("✅ Target AnimBP updated"));
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ Failed to setup dynamic retargeting"));
	}
}

void AGLBRetargetTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// GC 보호 해제
	if (LoadedTargetMesh && LoadedTargetMesh->IsRooted())
	{
		LoadedTargetMesh->RemoveFromRoot();
		PRINTLOG(TEXT("🧹 Cleaned up loaded mesh from GC protection"));
	}
	
	Super::EndPlay(EndPlayReason);
}

USkeletalMesh* AGLBRetargetTestActor::LoadGLBFile(const FString& FilePath)
{
	FString ContentDir = FPaths::ProjectContentDir();
	FString FullPath = FPaths::Combine(ContentDir, FilePath);
    
	PRINTLOG(TEXT("📁 Loading GLB from: %s"), *FullPath);
    
	if (!FPaths::FileExists(FullPath))
	{
		PRINTLOG(TEXT("❌ File not found: %s"), *FullPath);
		return nullptr;
	}
    
	// glTF Asset 로드
	UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
		FullPath, false, FglTFRuntimeConfig()
	);
    
	if (!Asset)
	{
		PRINTLOG(TEXT("❌ Failed to load glTF asset"));
		return nullptr;
	}
    
	// SkeletalMesh 설정
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
	SkeletalMeshConfig.bOverwriteRefSkeleton = true;
	SkeletalMeshConfig.MorphTargetsDuplicateStrategy = 
		EglTFRuntimeMorphTargetsDuplicateStrategy::Merge;
    
	// SkeletalMesh 생성
	USkeletalMesh* LoadedMesh = Asset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);
    
	if (LoadedMesh)
	{
		LoadedMesh->AddToRoot(); // GC 방지
		PRINTLOG(TEXT("✅ GLB mesh loaded successfully"));
		return LoadedMesh;
	}
    
	PRINTLOG(TEXT("❌ Failed to create skeletal mesh from GLB"));
	return nullptr;
}

bool AGLBRetargetTestActor::SetupDynamicRetargeting()
{
	// 1. GLB 파일 로드
	LoadedTargetMesh = LoadGLBFile(GLBFilePath);
	if (!LoadedTargetMesh)
		return false;
    
	// 2. 타겟 메시 설정
	TargetMeshComponent->SetSkeletalMesh(LoadedTargetMesh);
    
	// 3. ✅ 미리 만든 타겟 IK Rig 사용 (범용 Humanoid IK Rig)
	if (!TargetIKRig)
	{
		PRINTLOG(TEXT("❌ Target IK Rig not set! Please assign a generic humanoid IK Rig"));
		return false;
	}
    
	// 4. ✅ 미리 만든 Retargeter 사용
	if (!RetargeterAsset)
	{
		PRINTLOG(TEXT("❌ Retargeter asset not set!"));
		return false;
	}
    
	// 5. IK Retarget Processor 초기화
	RetargetProcessor = NewObject<UIKRetargetProcessor>();
    
	FRetargetProfile Profile;
	RetargetProcessor->Initialize(
		SourceMeshComponent->GetSkeletalMeshAsset(),
		LoadedTargetMesh,  // 런타임 로드된 메시
		RetargeterAsset,   // 미리 만든 Retargeter
		Profile
	);
    
	// 6. AnimBP 설정
	if (TargetAnimBPClass)
	{
		TargetMeshComponent->SetAnimInstanceClass(TargetAnimBPClass);
	}
    
	return true;
}


void AGLBRetargetTestActor::PrintBoneStructure(USkeletalMesh* Mesh, const FString& MeshName)
{
	if (!Mesh)
		return;
        
	const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
	PRINTLOG(TEXT("=== %s Bone Structure ==="), *MeshName);
	PRINTLOG(TEXT("Total Bones: %d"), RefSkeleton.GetNum());
    
	// 처음 10개 본만 출력 (간결하게)
	int32 MaxBones = FMath::Min(10, RefSkeleton.GetNum());
	for (int32 i = 0; i < MaxBones; i++)
	{
		FName BoneName = RefSkeleton.GetBoneName(i);
		int32 ParentIndex = RefSkeleton.GetParentIndex(i);
		PRINTLOG(TEXT("  [%d] %s (Parent: %d)"), i, *BoneName.ToString(), ParentIndex);
	}
    
	if (RefSkeleton.GetNum() > 10)
	{
		PRINTLOG(TEXT("  ... and %d more bones"), RefSkeleton.GetNum() - 10);
	}
}
