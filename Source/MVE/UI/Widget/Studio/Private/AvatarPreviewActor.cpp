#include "AvatarPreviewActor.h"
#include "MVE.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "glTFRuntimeFunctionLibrary.h"

AAvatarPreviewActor::AAvatarPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Scene Capture Component만 생성
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	RootComponent = SceneCapture;
	
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	
	// CompositeMode 설정 (검은색 투명 문제 해결)
	SceneCapture->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
	
	SceneCapture->bAlwaysPersistRenderingState = true;
	
	// ShowFlags 설정 (더 나은 렌더링)
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetVolumetricFog(false);
	SceneCapture->ShowFlags.SetMotionBlur(false);
	SceneCapture->ShowFlags.SetLighting(true);
	SceneCapture->ShowFlags.SetDynamicShadows(true);
	SceneCapture->ShowFlags.SetPostProcessing(true);
	SceneCapture->ShowFlags.SetAntiAliasing(true);
	SceneCapture->ShowFlags.SetTemporalAA(true);
	SceneCapture->ShowFlags.SetAmbientOcclusion(true);
}

void AAvatarPreviewActor::BeginPlay()
{
	Super::BeginPlay();
	CreateRenderTarget();
	
	PRINTLOG(TEXT("=== AvatarPreviewActor BeginPlay ==="));
	PRINTLOG(TEXT("RenderTarget: %s"), RenderTarget ? TEXT("OK") : TEXT("NULL"));
	PRINTLOG(TEXT("SceneCapture: %s"), SceneCapture ? TEXT("OK") : TEXT("NULL"));
}

void AAvatarPreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAvatarPreviewActor::CreateRenderTarget()
{
	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		
		// ⭐ 명시적으로 포맷 설정 (검은색 투명 문제 해결의 핵심!)
		RenderTarget->RenderTargetFormat = RTF_RGBA8;
		
		// ⭐ 배경색 설정: 검은색이지만 완전 불투명 (Alpha=1.0)
		RenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		// 해상도 설정
		RenderTarget->InitAutoFormat(1024, 1024);
		
		// 추가 설정
		RenderTarget->bAutoGenerateMips = false;
		RenderTarget->SRGB = true;
		
		// 리소스 즉시 업데이트
		RenderTarget->UpdateResourceImmediate(true);

		PRINTLOG(TEXT("RenderTarget created: 1024x1024, Format=RGBA8, Alpha=1.0"));
	}

	if (SceneCapture)
	{
		SceneCapture->TextureTarget = RenderTarget;
	}
}

void AAvatarPreviewActor::LoadAvatarMesh(const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		PRINTLOG(TEXT("GLB file not found: %s"), *FilePath);
		
		// 파일이 없으면 테스트 메쉬 표시
		UStaticMesh* TestMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (TestMesh)
		{
			SetPreviewMesh(TestMesh);
			PRINTLOG(TEXT("Showing test cube instead"));
		}
		return;
	}

	// GLB 파일 로드 설정
	FglTFRuntimeConfig LoaderConfig;
	LoaderConfig.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;

	// GLB 파일 로드
	UglTFRuntimeAsset* GLTFAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
		FilePath,
		false,
		LoaderConfig
	);

	if (!GLTFAsset)
	{
		PRINTLOG(TEXT("Failed to load GLB file: %s"), *FilePath);
		
		// 로드 실패 시 테스트 메쉬
		UStaticMesh* TestMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		if (TestMesh)
		{
			SetPreviewMesh(TestMesh);
			PRINTLOG(TEXT("Showing test sphere instead"));
		}
		return;
	}

	LoadedGLTFAsset = GLTFAsset;

	// StaticMesh로 로드 시도
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;
	UStaticMesh* LoadedMesh = GLTFAsset->LoadStaticMesh(0, StaticMeshConfig);

	if (LoadedMesh)
	{
		SetPreviewMesh(LoadedMesh);
		PRINTLOG(TEXT("GLB loaded as StaticMesh: %s"), *FilePath);
		return;
	}

	// SkeletalMesh로 로드 시도
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
	USkeletalMesh* SkeletalMesh = GLTFAsset->LoadSkeletalMesh(0, 0, SkeletalMeshConfig);

	if (SkeletalMesh)
	{
		SetPreviewSkeletalMesh(SkeletalMesh);
		PRINTLOG(TEXT("GLB loaded as SkeletalMesh: %s"), *FilePath);
		return;
	}

	PRINTLOG(TEXT("Failed to load mesh from GLB: %s"), *FilePath);
}

void AAvatarPreviewActor::SetPreviewMesh(UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		PRINTLOG(TEXT("Mesh is null"));
		return;
	}

	// 기존 액터 삭제
	if (PreviewMeshActor)
	{
		PreviewMeshActor->Destroy();
		PreviewMeshActor = nullptr;
	}

	// 새 액터 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	PreviewMeshActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (PreviewMeshActor)
	{
		// StaticMeshComponent 추가
		UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(PreviewMeshActor);
		MeshComp->SetStaticMesh(Mesh);
		MeshComp->SetCastShadow(true);
		MeshComp->RegisterComponent();
		PreviewMeshActor->SetRootComponent(MeshComp);

		// TargetActor 설정
		TargetActor = PreviewMeshActor;

		// ShowOnlyList에 추가
		if (SceneCapture)
		{
			SceneCapture->ShowOnlyComponents.Empty();
			SceneCapture->ShowOnlyComponents.Add(MeshComp);
			SceneCapture->CaptureScene();
			
			PRINTLOG(TEXT("ShowOnlyComponents count: %d"), SceneCapture->ShowOnlyComponents.Num());
		}

		PRINTLOG(TEXT("Preview mesh set: %s"), *Mesh->GetName());
	}
	else
	{
		PRINTLOG(TEXT("Failed to spawn PreviewMeshActor"));
	}
}

void AAvatarPreviewActor::SetPreviewSkeletalMesh(USkeletalMesh* SkeletalMesh)
{
	if (!SkeletalMesh)
	{
		PRINTLOG(TEXT("SkeletalMesh is null"));
		return;
	}

	// 기존 액터 삭제
	if (PreviewMeshActor)
	{
		PreviewMeshActor->Destroy();
		PreviewMeshActor = nullptr;
	}

	// 새 액터 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	PreviewMeshActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (PreviewMeshActor)
	{
		// SkeletalMeshComponent 추가
		USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(PreviewMeshActor);
		MeshComp->SetSkeletalMesh(SkeletalMesh);
		MeshComp->SetCastShadow(true);
		MeshComp->RegisterComponent();
		PreviewMeshActor->SetRootComponent(MeshComp);

		// TargetActor 설정
		TargetActor = PreviewMeshActor;

		// ShowOnlyList에 추가
		if (SceneCapture)
		{
			SceneCapture->ShowOnlyComponents.Empty();
			SceneCapture->ShowOnlyComponents.Add(MeshComp);
			SceneCapture->CaptureScene();
		}

		PRINTLOG(TEXT("Preview skeletal mesh set: %s"), *SkeletalMesh->GetName());
	}
	else
	{
		PRINTLOG(TEXT("Failed to spawn PreviewMeshActor"));
	}
}