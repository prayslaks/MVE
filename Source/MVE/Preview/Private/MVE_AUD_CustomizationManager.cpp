#include "../Public/MVE_AUD_CustomizationManager.h"
#include "DesktopPlatformModule.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeParser.h"
#include "HttpModule.h"
#include "IDesktopPlatform.h"
#include "MVE.h"
#include "API/Public/MVE_API_Helper.h"
#include "API/Public/MVE_API_ResponseData.h"
#include "API/Public/MVE_HTTP_Client.h"
#include "MVE_AUD_PreviewCameraPawn.h"
#include "MVE_AUD_PreviewCaptureActor.h"
#include "MVE_AUD_WidgetClass_PreviewWidget.h"
#include "MVE_GM_PreviewMesh.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


void UMVE_AUD_CustomizationManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PRINTLOG(TEXT("=== CustomizationManager Initialize ==="));

	MeshRenderTarget = LoadObject<UTextureRenderTarget2D>(nullptr,
		TEXT("/Game/Blueprints/Preview/RT_Preview.RT_Preview"));

	if (!MeshRenderTarget)
	{
		PRINTLOG(TEXT("❌ Failed to load Mesh Render Target: /Game/Blueprints/Preview/RT_Preview.RT_Preview"));
	}
	else
	{
		PRINTLOG(TEXT("✅ Mesh Render Target loaded successfully"));
	}
}

FString UMVE_AUD_CustomizationManager::OpenReferenceImageDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		PRINTLOG(TEXT("Desktop Platform not available"));
		return TEXT("");
	}

	TArray<FString> OutFiles;
	const FString Filter = TEXT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg");
    
	if (DesktopPlatform->OpenFileDialog(
		nullptr,
		TEXT("참고 이미지 선택"),
		FPaths::ProjectContentDir(),
		TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OutFiles))
	{
		if (OutFiles.Num() > 0)
		{
			if (LoadReferenceImage(OutFiles[0]))
			{
				return ReferenceImageFileName;
			}
		}
	}

	return TEXT("");
}

void UMVE_AUD_CustomizationManager::RequestModelGeneration(const FString& PromptText, const FString& ImagePath)
{
	PRINTLOG(TEXT("=== RequestModelGeneration ==="));
	PRINTLOG(TEXT("Prompt: %s"), *PromptText);

	// 프롬프트 검증
	if (PromptText.IsEmpty())
	{
		PRINTLOG(TEXT("❌ Prompt text is empty"));
		return;
	}

	// 이미지 경로 결정 (매개변수로 전달된 경로 우선, 없으면 저장된 경로 사용)
	FString FinalImagePath = ImagePath.IsEmpty() ? ReferenceImageFilePath : ImagePath;

	if (FinalImagePath.IsEmpty())
	{
		PRINTLOG(TEXT("⚠️ No reference image provided, generating without image"));
	}
	else
	{
		PRINTLOG(TEXT("Image Path: %s"), *FinalImagePath);
	}

	// MVE_API_Helper::GenerateModel 호출
	FOnGenerateModelComplete OnComplete;
	OnComplete.BindUObject(this, &UMVE_AUD_CustomizationManager::OnGenerateModelComplete);

	UMVE_API_Helper::GenerateModel(PromptText, FinalImagePath, OnComplete);
	PRINTLOG(TEXT("✅ Model generation request sent via MVE_API_Helper"));
}

bool UMVE_AUD_CustomizationManager::LoadReferenceImage(const FString& FilePath)
{
	// 기존 데이터 클리어
	ReferenceImageData.Empty();
	ReferenceImageFileName.Empty();
	ReferenceImageFormat.Empty();
	ReferenceImageFilePath.Empty();

	// 파일을 바이트 배열로 읽기
	if (!FFileHelper::LoadFileToArray(ReferenceImageData, *FilePath))
	{
		PRINTLOG(TEXT("Failed to load file: %s"), *FilePath);
		return false;
	}

	// 파일 크기 제한 (10MB)
	const int32 MaxFileSizeBytes = 10 * 1024 * 1024;
	if (ReferenceImageData.Num() > MaxFileSizeBytes)
	{
		PRINTLOG(TEXT("File too large: %d bytes"), ReferenceImageData.Num());
		ReferenceImageData.Empty();
		return false;
	}

	// 파일명과 확장자 저장
	ReferenceImageFileName = FPaths::GetCleanFilename(FilePath);
	ReferenceImageFormat = FPaths::GetExtension(FilePath).ToLower();
	ReferenceImageFilePath = FilePath;  // 파일 경로 저장

	// 지원 포맷 확인
	if (ReferenceImageFormat != TEXT("png") &&
		ReferenceImageFormat != TEXT("jpg") &&
		ReferenceImageFormat != TEXT("jpeg") &&
		ReferenceImageFormat != TEXT("gif"))
	{
		PRINTLOG(TEXT("Unsupported format: %s"), *ReferenceImageFormat);
		ReferenceImageData.Empty();
		ReferenceImageFileName.Empty();
		ReferenceImageFormat.Empty();
		ReferenceImageFilePath.Empty();
		return false;
	}

	return true;
}


void UMVE_AUD_CustomizationManager::OnGenerateModelComplete(bool bSuccess, const FGenerateModelResponseData& ResponseData, const FString& ErrorCode)
{
	PRINTLOG(TEXT("=== OnGenerateModelComplete ==="));

	if (bSuccess)
	{
		PRINTLOG(TEXT("✅ Model generation job created successfully"));
		PRINTLOG(TEXT("   Job ID: %s"), *ResponseData.JobId);
		PRINTLOG(TEXT("   Success: %s"), ResponseData.Success ? TEXT("true") : TEXT("false"));
		PRINTLOG(TEXT("   Code: %s"), *ResponseData.Code);
		PRINTLOG(TEXT("   Message: %s"), *ResponseData.Message);

		// JobId 저장
		CurrentJobId = ResponseData.JobId;

		// 기존 타이머가 있으면 중지
		if (UWorld* World = GetWorld())
		{
			if (ModelStatusCheckTimer.IsValid())
			{
				World->GetTimerManager().ClearTimer(ModelStatusCheckTimer);
			}

			// 상태 확인 타이머 시작 (2초마다)
			World->GetTimerManager().SetTimer(
				ModelStatusCheckTimer,
				this,
				&UMVE_AUD_CustomizationManager::CheckModelGenerationStatus,
				StatusCheckInterval,
				true  // 반복
			);

			PRINTLOG(TEXT("⏱️ Status check timer started (interval: %.1f seconds)"), StatusCheckInterval);
		}
		else
		{
			PRINTLOG(TEXT("❌ Failed to get World for timer"));
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ Model generation request failed"));
		PRINTLOG(TEXT("   Error Code: %s"), *ErrorCode);
		PRINTLOG(TEXT("   Message: %s"), *ResponseData.Message);
	}
}

void UMVE_AUD_CustomizationManager::CheckModelGenerationStatus()
{
	PRINTLOG(TEXT("🔍 Checking model generation status for Job ID: %s"), *CurrentJobId);

	if (CurrentJobId.IsEmpty())
	{
		PRINTLOG(TEXT("❌ No JobId to check"));
		return;
	}

	// GetModelGenerationStatus API 호출
	FOnGetJobStatusComplete OnComplete;
	OnComplete.BindUObject(this, &UMVE_AUD_CustomizationManager::OnGetModelStatusComplete);

	UMVE_API_Helper::GetModelGenerationStatus(CurrentJobId, OnComplete);
}

void UMVE_AUD_CustomizationManager::OnGetModelStatusComplete(bool bSuccess, const FGetJobStatusResponseData& ResponseData, const FString& ErrorCode)
{
	PRINTLOG(TEXT("=== OnGetModelStatusComplete ==="));

	if (!bSuccess)
	{
		PRINTLOG(TEXT("❌ Failed to get job status"));
		PRINTLOG(TEXT("   Error Code: %s"), *ErrorCode);

		// 타이머 중지
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ModelStatusCheckTimer);
			PRINTLOG(TEXT("⏹️ Status check timer stopped"));
		}
		return;
	}

	const FAIJobStatus& JobStatus = ResponseData.Data;
	PRINTLOG(TEXT("📊 Job Status: %s"), *JobStatus.Status);
	PRINTLOG(TEXT("   Job ID: %s"), *JobStatus.JobId);
	PRINTLOG(TEXT("   Prompt: %s"), *JobStatus.Prompt);
	PRINTLOG(TEXT("   Created At: %s"), *JobStatus.CreatedAt);

	// 상태에 따라 처리
	if (JobStatus.Status.Equals(TEXT("completed"), ESearchCase::IgnoreCase))
	{
		PRINTLOG(TEXT("✅ Model generation completed!"));
		PRINTLOG(TEXT("   Model ID: %d"), JobStatus.ModelId);
		PRINTLOG(TEXT("   Download URL: %s"), *JobStatus.DownloadUrl);

		// 타이머 중지
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ModelStatusCheckTimer);
			PRINTLOG(TEXT("⏹️ Status check timer stopped"));
		}

		// ⭐ 중요: 원격 URL 저장 (서버에 저장할 때 사용)
		CurrentRemoteURL = JobStatus.DownloadUrl;
		PRINTLOG(TEXT("💾 Remote URL saved: %s"), *CurrentRemoteURL);

		// 다운로드 경로 설정 (Saved/DownloadedModels 폴더에 저장)
		FString SaveDir = FPaths::ProjectSavedDir() / TEXT("DownloadedModels");

		// 디렉토리 생성
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*SaveDir))
		{
			PlatformFile.CreateDirectoryTree(*SaveDir);
			PRINTLOG(TEXT("✅ Created directory: %s"), *SaveDir);
		}

		FString SavePath = SaveDir / FString::Printf(TEXT("Model_%d.glb"), JobStatus.ModelId);

		PRINTLOG(TEXT("📥 Starting model download..."));
		PRINTLOG(TEXT("   Download URL: %s"), *JobStatus.DownloadUrl);
		PRINTLOG(TEXT("   Save path: %s"), *SavePath);

		// presigned URL로 직접 다운로드 (별도 API 호출 불필요)
		FOnHttpDownloadResult OnDownloadComplete;
		OnDownloadComplete.BindLambda([this, SavePath](bool bSuccess, const TArray<uint8>& FileData, const FString& ErrorMessage)
		{
			if (bSuccess && FileData.Num() > 0)
			{
				// 파일 저장
				if (FFileHelper::SaveArrayToFile(FileData, *SavePath))
				{
					PRINTLOG(TEXT("✅ Model downloaded successfully!"));
					PRINTLOG(TEXT("   File path: %s"), *SavePath);
					PRINTLOG(TEXT("   File size: %.2f MB"), FileData.Num() / (1024.0f * 1024.0f));

					// CurrentGLBFilePath에 저장
					CurrentGLBFilePath = SavePath;

					// 프리뷰 시작
					if (MeshPreviewWidget)
					{
						StartMeshPreview(SavePath, MeshPreviewWidget);
					}
				}
				else
				{
					PRINTLOG(TEXT("❌ Failed to save file: %s"), *SavePath);
				}
			}
			else
			{
				PRINTLOG(TEXT("❌ Model download failed: %s"), *ErrorMessage);
			}
		});

		// S3 presigned URL은 이미 서명이 포함되어 있으므로 Authorization 헤더 불필요
		FMVE_HTTP_Client::DownloadFile(JobStatus.DownloadUrl, TEXT(""), OnDownloadComplete);
	}
	else if (JobStatus.Status.Equals(TEXT("failed"), ESearchCase::IgnoreCase))
	{
		PRINTLOG(TEXT("❌ Model generation failed"));
		PRINTLOG(TEXT("   Error: %s"), *JobStatus.ErrorMessage);

		// 타이머 중지
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ModelStatusCheckTimer);
			PRINTLOG(TEXT("⏹️ Status check timer stopped"));
		}
	}
	else if (JobStatus.Status.Equals(TEXT("pending"), ESearchCase::IgnoreCase) ||
	         JobStatus.Status.Equals(TEXT("processing"), ESearchCase::IgnoreCase))
	{
		PRINTLOG(TEXT("⏳ Model is still being generated... (status: %s)"), *JobStatus.Status);
		// 타이머가 계속 실행되어 다시 확인
	}
	else
	{
		PRINTLOG(TEXT("⚠️ Unknown status: %s"), *JobStatus.Status);
	}
}

void UMVE_AUD_CustomizationManager::OnModelDownloadComplete(bool bSuccess, const FGetModelDownloadUrlResponseData& Data,const FString& SavedPath)
{
	PRINTLOG(TEXT("=== OnModelDownloadComplete ==="));

	if (bSuccess)
	{
		PRINTLOG(TEXT("✅ Model downloaded successfully!"));
		PRINTLOG(TEXT("   File path: %s"), *SavedPath);

		// 파일 존재 확인
		if (FPaths::FileExists(SavedPath))
		{
			int64 FileSize = IFileManager::Get().FileSize(*SavedPath);
			PRINTLOG(TEXT("   File size: %.2f MB"), FileSize / (1024.0f * 1024.0f));

			// CurrentGLBFilePath에 저장 (다른 함수들에서 사용할 수 있도록)
			CurrentGLBFilePath = SavedPath;

			PRINTLOG(TEXT("🎉 Model is ready to use!"));
			PRINTLOG(TEXT("   You can now call StartMeshPreview() to preview the model"));
			
			if (MeshPreviewWidget)
			{
			    StartMeshPreview(SavedPath, MeshPreviewWidget);
			}
		}
		else
		{
			PRINTLOG(TEXT("⚠️ File exists check failed: %s"), *SavedPath);
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ Model download failed"));
		PRINTLOG(TEXT("   Error: %s"), *SavedPath);  // SavedPath contains error message on failure
	}
}

AActor* UMVE_AUD_CustomizationManager::GetPreviewCharacter() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AMVE_GM_PreviewMesh* GM = World->GetAuthGameMode<AMVE_GM_PreviewMesh>();
	if (GM)
	{
		return GM->GetPreviewCharacter();
	}

	return nullptr;
}

void UMVE_AUD_CustomizationManager::AttachMeshToSocket(const FName& SocketName)
{
	AActor* PreviewCharacterActor = GetPreviewCharacter();

	if (!PreviewedMesh || !PreviewCharacterActor)
    {
        PRINTLOG(TEXT("❌ No mesh or character to attach"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // 기존 액세서리 제거
    if (AttachedMesh)
    {
        AttachedMesh->Destroy();
        AttachedMesh = nullptr;
    }

    // 새 액세서리 생성
    AActor* NewAccessory = World->SpawnActor<AActor>(
        PreviewedMesh->GetClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );

    if (!NewAccessory)
    {
        PRINTLOG(TEXT("❌ Failed to spawn accessory"));
        return;
    }

    // 메시 컴포넌트 복사
    UStaticMeshComponent* SourceMesh = PreviewedMesh->FindComponentByClass<UStaticMeshComponent>();
    UStaticMeshComponent* NewMesh = NewObject<UStaticMeshComponent>(NewAccessory);
    
    if (!SourceMesh || !NewMesh)
    {
        PRINTLOG(TEXT("❌ Failed to create mesh component"));
        NewAccessory->Destroy();
        return;
    }

    FVector DesiredScale = SourceMesh->GetComponentScale();
    
    PRINTLOG(TEXT("=== Attaching Accessory ==="));
    PRINTLOG(TEXT("Desired Scale: %s"), *DesiredScale.ToString());

    NewMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
    
    // 머티리얼 복사
    for (int32 i = 0; i < SourceMesh->GetNumMaterials(); i++)
    {
        NewMesh->SetMaterial(i, SourceMesh->GetMaterial(i));
    }
    
    NewAccessory->SetRootComponent(NewMesh);
    NewMesh->RegisterComponent();

    // 소켓에 부착
    USkeletalMeshComponent* SkelMesh = PreviewCharacterActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkelMesh)
    {
        PRINTLOG(TEXT("❌ No skeletal mesh found on character"));
        NewAccessory->Destroy();
        return;
    }

    if (SkelMesh->DoesSocketExist(SocketName))
    {
        // 커스텀 부착 규칙
        FAttachmentTransformRules CustomRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepWorld,
            false
        );
        
        NewAccessory->AttachToComponent(SkelMesh, CustomRules, SocketName);

        // 스케일 복원
        NewMesh->SetWorldScale3D(DesiredScale);

        PRINTLOG(TEXT("✅ Accessory attached to socket: %s"), *SocketName.ToString());

    	//ScaleMeshToCharacter();
    }
    else
    {
        PRINTLOG(TEXT("⚠️ Socket not found: %s"), *SocketName.ToString());
        NewAccessory->Destroy();
        return;
    }

	AttachedMesh = NewAccessory;

	// 커스터마이징 데이터 저장 (수정된 부분)
	SavedCustomization.ModelUrl = CurrentRemoteURL;  // GLBFilePath → ModelUrl
	SavedCustomization.SocketName = SocketName.ToString();  // FName → FString
	
	// Transform을 분해해서 저장
	FTransform RelativeTransform = NewAccessory->GetTransform().GetRelativeTransform(SkelMesh->GetComponentTransform());
	SavedCustomization.RelativeLocation = RelativeTransform.GetLocation();
	SavedCustomization.RelativeRotation = RelativeTransform.GetRotation().Rotator();
	SavedCustomization.RelativeScale = RelativeTransform.GetScale3D().X;  // Uniform Scale 가정

	PRINTLOG(TEXT("✅ Customization data saved:"));
	PRINTLOG(TEXT("   Model URL: %s"), *SavedCustomization.ModelUrl);
	PRINTLOG(TEXT("   Socket: %s"), *SavedCustomization.SocketName);
	PRINTLOG(TEXT("   Location: %s"), *SavedCustomization.RelativeLocation.ToString());
	PRINTLOG(TEXT("   Rotation: %s"), *SavedCustomization.RelativeRotation.ToString());
	PRINTLOG(TEXT("   Scale: %.2f"), SavedCustomization.RelativeScale);

    // 자동으로 기즈모 모드 전환
    UWorld* CurrentWorld = GetWorld();
    if (CurrentWorld)
    {
        APlayerController* PC = CurrentWorld->GetFirstPlayerController();
        if (PC)
        {
            AMVE_AUD_PreviewCameraPawn* CameraPawn = Cast<AMVE_AUD_PreviewCameraPawn>(PC->GetPawn());
            if (CameraPawn)
            {
                PRINTLOG(TEXT("🎯 Switching to Gizmo mode via CameraPawn..."));
                CameraPawn->SwitchToGizmoMode(AttachedMesh);
            }
        }
    }
}

void UMVE_AUD_CustomizationManager::RemoveMesh()
{
	if (AttachedMesh)
	{
		AttachedMesh->Destroy();
		AttachedMesh = nullptr;
		PRINTLOG(TEXT("✅ Accessory removed"));
	}
}

void UMVE_AUD_CustomizationManager::StartMeshPreview(const FString& GLBFilePath,
                                                     UMVE_AUD_WidgetClass_PreviewWidget* InPreviewWidget)
{
	PRINTLOG(TEXT("=== StartMeshPreview called ==="));
	PRINTLOG(TEXT("GLB Path: %s"), *GLBFilePath);

	StopMeshPreview();

	// GLB 파일 경로 저장
	CurrentGLBFilePath = GLBFilePath;

	MeshPreviewWidget = InPreviewWidget;

	if (!MeshPreviewWidget)
	{
		PRINTLOG(TEXT("❌ MeshPreviewWidget is null"));
		return;
	}

	if (!MeshRenderTarget)
	{
		PRINTLOG(TEXT("❌ MeshRenderTarget is null"));
		return;
	}

	PRINTLOG(TEXT("✅ MeshPreviewWidget and MeshRenderTarget are valid"));

	UWorld* World = GetWorld();
	if (!World)
	{
		PRINTLOG(TEXT("❌ World is null"));
		return;
	}

	MeshCaptureActor = World->SpawnActor<AMVE_AUD_PreviewCaptureActor>();
	if (!MeshCaptureActor)
	{
		PRINTLOG(TEXT("❌ Failed to spawn MeshCaptureActor"));
		return;
	}

	PRINTLOG(TEXT("✅ MeshCaptureActor spawned successfully"));

	MeshCaptureActor->RenderTarget = MeshRenderTarget;

	if (GLBFilePath == FString(""))
	{
		UClass* ActorClass = LoadObject<UClass>(nullptr,
		TEXT("/Game/Blueprints/Preview/BP_EmptyActor.BP_EmptyActor_C"));
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass);
		OnMeshLoaded(SpawnedActor);
	}
	else
	{
		// GLB 파일 로딩 (비동기)
		PRINTLOG(TEXT("Starting GLB file loading..."));
		LoadMeshFromGLB(GLBFilePath, [this](AActor* LoadedActor)
		{
			OnMeshLoaded(LoadedActor);
		});
	}
}

void UMVE_AUD_CustomizationManager::StopMeshPreview()
{
	// Scene Capture Actor 삭제
	if (MeshCaptureActor)
	{
		MeshCaptureActor->Destroy();
		MeshCaptureActor = nullptr;
	}
    
	// 프리뷰 액세서리 삭제
	if (PreviewedMesh)
	{
		PreviewedMesh->Destroy();
		PreviewedMesh = nullptr;
	}
    
	MeshPreviewWidget = nullptr;
}

void UMVE_AUD_CustomizationManager::SaveAccessoryTransform(AActor* Accessory, const FTransform& NewTransform)
{
	if (!Accessory)
	{
		return;
	}
    
	SavedTransforms.Add(Accessory, NewTransform);
}

AActor* UMVE_AUD_CustomizationManager::SpawnPreviewCharacter()
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	// 방법 1: 프리셋 캐릭터 블루프린트 로드
	UClass* CharacterClass = LoadClass<AActor>(nullptr, 
		TEXT("/Game/Blueprints/Character/BP_ClientCharacter.BP_ClientCharacter_C"));
    
	if (!CharacterClass)
	{
		PRINTLOG(TEXT("❌ Failed to load preview character class"));
		return nullptr;
	}

	AActor* Character = World->SpawnActor<AActor>(
		CharacterClass,
		FVector(-10000, 0, 0),  // 화면 밖
		FRotator::ZeroRotator
	);

	if (Character)
	{
		PRINTLOG(TEXT("✅ Preview character spawned"));
	}

	return Character;
}

void UMVE_AUD_CustomizationManager::OnMeshLoaded(AActor* LoadedActor)
{
	PRINTLOG(TEXT("=== OnMeshLoaded called ==="));

	if (!LoadedActor)
	{
		PRINTLOG(TEXT("❌ LoadedActor is null"));
		return;
	}

	if (!MeshCaptureActor)
	{
		PRINTLOG(TEXT("❌ MeshCaptureActor is null"));
		return;
	}

	if (!MeshPreviewWidget)
	{
		PRINTLOG(TEXT("❌ MeshPreviewWidget is null"));
		return;
	}

	PRINTLOG(TEXT("✅ All components valid, setting up preview"));

	PreviewedMesh = LoadedActor;

	// 1. 메시 배치 (화면 밖 - CharacterPreview와 다른 위치)
	FVector MeshLocation = FVector(-10000, 10000, 0);  // 캐릭터 프리뷰(-10000, 0, 0)와 겹치지 않게
	PreviewedMesh->SetActorLocation(MeshLocation);
	PreviewedMesh->SetActorRotation(FRotator::ZeroRotator);
	PRINTLOG(TEXT("✅ Mesh positioned at: %s"), *MeshLocation.ToString());

	// ⭐ 캐릭터 크기에 맞게 메시 스케일 자동 조정
	ScaleMeshToCharacter();
	
	// 2. Scene Capture 타겟 설정
	MeshCaptureActor->SetCaptureTarget(PreviewedMesh);
	PRINTLOG(TEXT("✅ Capture target set"));

	// 3. 카메라 초기 위치 (메시 기준 상대 위치)
	FVector CameraLocation = MeshLocation + FVector(150, 0, 50);
	MeshCaptureActor->SetActorLocation(CameraLocation);
	MeshCaptureActor->SetActorRotation((MeshLocation - CameraLocation).Rotation());
	PRINTLOG(TEXT("✅ Camera positioned at: %s"), *CameraLocation.ToString());

	// 4. UI 위젯에 Render Target 연결
	MeshPreviewWidget->SetRenderTarget(MeshRenderTarget);
	PRINTLOG(TEXT("✅ Render target connected to widget"));

	// 5. UI 위젯에 CaptureActor 연결 (마우스 컨트롤용)
	MeshPreviewWidget->SetCaptureActor(MeshCaptureActor);
	PRINTLOG(TEXT("✅ Capture actor connected to widget"));

	// 6. 자동 카메라 거리 조정
	AutoAdjustCameraDistance();
	PRINTLOG(TEXT("✅ Mesh preview setup complete"));
}

void UMVE_AUD_CustomizationManager::AutoAdjustCameraDistance()
{
	PRINTLOG(TEXT("=== AutoAdjustCameraDistance called ==="));

	if (!PreviewedMesh)
	{
		PRINTLOG(TEXT("❌ PreviewedMesh is null"));
		return;
	}

	if (!MeshCaptureActor)
	{
		PRINTLOG(TEXT("❌ MeshCaptureActor is null"));
		return;
	}

	if (!MeshPreviewWidget)
	{
		PRINTLOG(TEXT("❌ MeshPreviewWidget is null"));
		return;
	}

	// 액세서리의 바운딩 박스 계산
	FVector Origin, BoxExtent;
	PreviewedMesh->GetActorBounds(false, Origin, BoxExtent);

	PRINTLOG(TEXT("Mesh bounds - Origin: %s, Extent: %s"), *Origin.ToString(), *BoxExtent.ToString());

	// 바운딩 박스 크기에 따라 카메라 거리 자동 조정
	float MaxExtent = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z);
	float OptimalDistance = FMath::Clamp(MaxExtent * 2.5f, 100.0f, 10000.0f);

	PRINTLOG(TEXT("MaxExtent: %f, OptimalDistance: %f"), MaxExtent, OptimalDistance);

	// PreviewWidget의 초기 거리 설정
	MeshPreviewWidget->SetInitialDistance(OptimalDistance);

	PRINTLOG(TEXT("✅ Camera distance adjusted"));
}

void UMVE_AUD_CustomizationManager::LoadMeshFromGLB(const FString& GLBFilePath,
	TFunction<void(AActor*)> OnLoadComplete)
{
	PRINTLOG(TEXT("=== LoadMeshFromGLB called ==="));
	PRINTLOG(TEXT("File path: %s"), *GLBFilePath);

	// 파일 존재 확인
	if (!FPaths::FileExists(GLBFilePath))
	{
		PRINTLOG(TEXT("❌ GLB file does not exist: %s"), *GLBFilePath);
		return;
	}

	PRINTLOG(TEXT("✅ GLB file exists"));

	// 콜백 저장
	LoadCompleteCallback = OnLoadComplete;

	// glTFRuntime으로 GLB 파일 로딩
	FglTFRuntimeConfig LoaderConfig;
	LoaderConfig.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;

	PRINTLOG(TEXT("Loading GLB asset..."));

	// 파일에서 GLB 로드
	UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
		GLBFilePath,
		false,  // bPathRelativeToContent
		LoaderConfig
	);

	if (!Asset)
	{
		PRINTLOG(TEXT("❌ Failed to load GLB file: %s"), *GLBFilePath);
		return;
	}

	PRINTLOG(TEXT("✅ GLB asset loaded, processing nodes..."));

	// 비동기 로딩이므로 콜백 연결
	OnGLTFAssetLoaded(Asset);
}

void UMVE_AUD_CustomizationManager::OnGLTFAssetLoaded(UglTFRuntimeAsset* Asset)
{
	PRINTLOG(TEXT("=== OnGLTFAssetLoaded called ==="));

	if (!Asset)
	{
		PRINTLOG(TEXT("❌ Asset is null in OnGLTFAssetLoaded"));
		return;
	}

	if (!GetWorld())
	{
		PRINTLOG(TEXT("❌ World is null in OnGLTFAssetLoaded"));
		return;
	}

	PRINTLOG(TEXT("✅ Asset and World are valid"));

	// 스켈레탈 메시 설정
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
	SkeletalMeshConfig.bOverwriteRefSkeleton = true;

	// GLB에서 첫 번째 메시 추출
	TArray<FglTFRuntimeNode> Nodes = Asset->GetNodes();

	PRINTLOG(TEXT("Number of nodes in GLB: %d"), Nodes.Num());

	if (Nodes.Num() == 0)
	{
		PRINTLOG(TEXT("❌ No nodes found in GLB asset"));
		return;
	}

	// 액세서리 액터 생성
	AActor* MeshActor = GetWorld()->SpawnActor<AActor>();

	if (!MeshActor)
	{
		PRINTLOG(TEXT("❌ Failed to spawn mesh actor"));
		return;
	}

	PRINTLOG(TEXT("✅ Mesh actor spawned"));

	// Static Mesh Component 추가
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(MeshActor);

	if (!MeshComponent)
	{
		PRINTLOG(TEXT("❌ Failed to create mesh component"));
		MeshActor->Destroy();
		return;
	}

	MeshActor->SetRootComponent(MeshComponent);
	MeshComponent->RegisterComponent();

	PRINTLOG(TEXT("✅ Mesh component created and registered"));

	// glTFRuntime으로 Static Mesh 생성
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;

	PRINTLOG(TEXT("Loading static mesh from GLB..."));

	UStaticMesh* StaticMesh = Asset->LoadStaticMesh(0, StaticMeshConfig);

	if (!StaticMesh)
	{
		PRINTLOG(TEXT("❌ Failed to load static mesh from GLB"));
		MeshActor->Destroy();
		return;
	}

	PRINTLOG(TEXT("✅ Static mesh loaded successfully"));

	// 메시 설정
	MeshComponent->SetStaticMesh(StaticMesh);

	PRINTLOG(TEXT("✅ Static mesh set to component"));
	PRINTLOG(TEXT("✅ GLB loaded successfully, invoking callback"));

	// 콜백 호출
	if (LoadCompleteCallback)
	{
		LoadCompleteCallback(MeshActor);
	}
	else
	{
		PRINTLOG(TEXT("⚠️ LoadCompleteCallback is null"));
	}
}

void UMVE_AUD_CustomizationManager::ScaleMeshToCharacter()
{
	AActor* PreviewCharacterActor = GetPreviewCharacter();

	if (!PreviewedMesh || !PreviewCharacterActor)
    {
        PRINTLOG(TEXT("❌ Missing mesh or character"));
        return;
    }
    
    // 1. 캐릭터 크기 확인
    FVector CharacterOrigin, CharacterExtent;
    PreviewCharacterActor->GetActorBounds(false, CharacterOrigin, CharacterExtent);
    float CharacterMaxSize = FMath::Max3(CharacterExtent.X, CharacterExtent.Y, CharacterExtent.Z) * 2.0f;
    
    PRINTLOG(TEXT("=== Size Comparison ==="));
    PRINTLOG(TEXT("Character Size: X=%.2f, Y=%.2f, Z=%.2f cm"), 
        CharacterExtent.X * 2.0f, CharacterExtent.Y * 2.0f, CharacterExtent.Z * 2.0f);
    PRINTLOG(TEXT("Character Max: %.2f cm"), CharacterMaxSize);
    
    // 2. 메시 크기 확인
    FVector MeshOrigin, MeshExtent;
    PreviewedMesh->GetActorBounds(false, MeshOrigin, MeshExtent);
    float MeshMaxSize = FMath::Max3(MeshExtent.X, MeshExtent.Y, MeshExtent.Z) * 2.0f;
    
    PRINTLOG(TEXT("Mesh Size: X=%.2f, Y=%.2f, Z=%.2f cm"), 
        MeshExtent.X * 2.0f, MeshExtent.Y * 2.0f, MeshExtent.Z * 2.0f);
    PRINTLOG(TEXT("Mesh Max: %.2f cm"), MeshMaxSize);
    
    // 3. 목표 크기 계산 (캐릭터의 MaxMeshSizeRatio% 크기)
    float TargetMaxSize = CharacterMaxSize * MaxMeshSizeRatio;
    
    PRINTLOG(TEXT("Target Max Size: %.2f cm (%.0f%% of character)"), 
        TargetMaxSize, MaxMeshSizeRatio * 100.0f);
    
    // 4. 메시가 목표 크기보다 크면 스케일 조정
    if (MeshMaxSize > TargetMaxSize)
    {
        float ScaleFactor = TargetMaxSize / MeshMaxSize;
        
        UStaticMeshComponent* MeshComp = PreviewedMesh->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            FVector CurrentScale = MeshComp->GetComponentScale();
            FVector NewScale = CurrentScale * ScaleFactor;
            MeshComp->SetWorldScale3D(NewScale);
            
            PRINTLOG(TEXT("✅ Mesh scaled down"));
            PRINTLOG(TEXT("   Scale Factor: %.3f"), ScaleFactor);
            PRINTLOG(TEXT("   Old Scale: %s"), *CurrentScale.ToString());
            PRINTLOG(TEXT("   New Scale: %s"), *NewScale.ToString());
            
            // 스케일 후 실제 크기 확인
            PreviewedMesh->GetActorBounds(false, MeshOrigin, MeshExtent);
            float NewMaxSize = FMath::Max3(MeshExtent.X, MeshExtent.Y, MeshExtent.Z) * 2.0f;
            PRINTLOG(TEXT("   New Mesh Max: %.2f cm"), NewMaxSize);
        }
    }
    else
    {
        PRINTLOG(TEXT("✅ Mesh size is acceptable (no scaling needed)"));
    }
    
    PRINTLOG(TEXT("======================"));
}

FVector UMVE_AUD_CustomizationManager::GetCharacterSize() const
{
	AActor* PreviewCharacterActor = GetPreviewCharacter();

	if (!PreviewCharacterActor)
	{
		return FVector::ZeroVector;
	}

	FVector Origin, BoxExtent;
	PreviewCharacterActor->GetActorBounds(false, Origin, BoxExtent);

	// 전체 크기 반환
	return BoxExtent * 2.0f;
}

FString UMVE_AUD_CustomizationManager::SerializeCustomizationData(const FCustomizationData& Data) const
{
	// JSON 배열 생성 (단일 액세서리지만 배열 형식)
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	
	// JSON 객체 생성
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// socketName
	JsonObject->SetStringField(TEXT("socketName"), Data.SocketName);
	
	// relativeLocation
	TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
	LocationObj->SetNumberField(TEXT("x"), Data.RelativeLocation.X);
	LocationObj->SetNumberField(TEXT("y"), Data.RelativeLocation.Y);
	LocationObj->SetNumberField(TEXT("z"), Data.RelativeLocation.Z);
	JsonObject->SetObjectField(TEXT("relativeLocation"), LocationObj);
	
	// relativeRotation
	TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
	RotationObj->SetNumberField(TEXT("pitch"), Data.RelativeRotation.Pitch);
	RotationObj->SetNumberField(TEXT("yaw"), Data.RelativeRotation.Yaw);
	RotationObj->SetNumberField(TEXT("roll"), Data.RelativeRotation.Roll);
	JsonObject->SetObjectField(TEXT("relativeRotation"), RotationObj);
	
	// relativeScale
	JsonObject->SetNumberField(TEXT("relativeScale"), Data.RelativeScale);
	
	// modelUrl
	JsonObject->SetStringField(TEXT("modelUrl"), Data.ModelUrl);
	
	// 배열에 추가
	JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	
	// JSON 문자열로 변환
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonArray, Writer);
	
	return OutputString;
}

FCustomizationData UMVE_AUD_CustomizationManager::DeserializeCustomizationData(const FString& JsonString) const
{
	FCustomizationData Result;
	
	// JSON 배열 파싱
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
	{
		PRINTLOG(TEXT("Failed to parse JSON array"));
		return Result;
	}
	
	// 객체 가져오기
	TSharedPtr<FJsonObject> JsonObject = JsonArray[0]->AsObject();
	if (!JsonObject.IsValid())
	{
		PRINTLOG(TEXT("Invalid JSON object"));
		return Result;
	}
	
	// socketName
	Result.SocketName = JsonObject->GetStringField(TEXT("socketName"));
	
	// relativeLocation
	const TSharedPtr<FJsonObject>* LocationObj;
	if (JsonObject->TryGetObjectField(TEXT("relativeLocation"), LocationObj))
	{
		Result.RelativeLocation.X = (*LocationObj)->GetNumberField(TEXT("x"));
		Result.RelativeLocation.Y = (*LocationObj)->GetNumberField(TEXT("y"));
		Result.RelativeLocation.Z = (*LocationObj)->GetNumberField(TEXT("z"));
	}
	
	// relativeRotation
	const TSharedPtr<FJsonObject>* RotationObj;
	if (JsonObject->TryGetObjectField(TEXT("relativeRotation"), RotationObj))
	{
		Result.RelativeRotation.Pitch = (*RotationObj)->GetNumberField(TEXT("pitch"));
		Result.RelativeRotation.Yaw = (*RotationObj)->GetNumberField(TEXT("yaw"));
		Result.RelativeRotation.Roll = (*RotationObj)->GetNumberField(TEXT("roll"));
	}
	
	// relativeScale
	Result.RelativeScale = JsonObject->GetNumberField(TEXT("relativeScale"));
	
	// modelUrl
	Result.ModelUrl = JsonObject->GetStringField(TEXT("modelUrl"));
	
	return Result;
}

void UMVE_AUD_CustomizationManager::SaveAccessoryPresetToServer(const FString& PresetName)
{
	PRINTLOG(TEXT("=== SaveAccessoryPresetToServer ==="));
    
    // 1. 저장된 커스터마이징 데이터 확인
    if (SavedCustomization.ModelUrl.IsEmpty())
    {
        PRINTLOG(TEXT("⚠️ No customization data to save"));
        return;
    }
    
    PRINTLOG(TEXT("✅ Saved customization data found"));
    PRINTLOG(TEXT("   Model URL: %s"), *SavedCustomization.ModelUrl);
    PRINTLOG(TEXT("   Socket: %s"), *SavedCustomization.SocketName);
    
    // 2. Accessories 배열 생성 (API 형식)
    TArray<TSharedPtr<FJsonValue>> AccessoriesArray;
    
    TSharedPtr<FJsonObject> AccessoryObject = MakeShareable(new FJsonObject);
    AccessoryObject->SetStringField(TEXT("socketName"), SavedCustomization.SocketName);
    
    // RelativeLocation
    TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
    LocationObj->SetNumberField(TEXT("x"), SavedCustomization.RelativeLocation.X);
    LocationObj->SetNumberField(TEXT("y"), SavedCustomization.RelativeLocation.Y);
    LocationObj->SetNumberField(TEXT("z"), SavedCustomization.RelativeLocation.Z);
    AccessoryObject->SetObjectField(TEXT("relativeLocation"), LocationObj);
    
    // RelativeRotation
    TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
    RotationObj->SetNumberField(TEXT("pitch"), SavedCustomization.RelativeRotation.Pitch);
    RotationObj->SetNumberField(TEXT("yaw"), SavedCustomization.RelativeRotation.Yaw);
    RotationObj->SetNumberField(TEXT("roll"), SavedCustomization.RelativeRotation.Roll);
    AccessoryObject->SetObjectField(TEXT("relativeRotation"), RotationObj);
    
    // RelativeScale
    AccessoryObject->SetNumberField(TEXT("relativeScale"), SavedCustomization.RelativeScale);
    
    // ModelUrl (PresignedURL)
    AccessoryObject->SetStringField(TEXT("modelUrl"), SavedCustomization.ModelUrl);
    
    // 배열에 추가
    AccessoriesArray.Add(MakeShareable(new FJsonValueObject(AccessoryObject)));
    
    PRINTLOG(TEXT("✅ Accessory data prepared for API"));
    
    // 3. API 호출 준비
    FOnSavePresetComplete OnResult;
    OnResult.BindUObject(this, &UMVE_AUD_CustomizationManager::HandleSavePresetComplete);
    
    // 4. ⭐ API Helper 호출
    UMVE_API_Helper::SaveAccessoryPreset(
        PresetName,              // PresetName
        AccessoriesArray,        // Accessories (JSON 배열)
        TEXT(""),                // Description (선택)
        false,                   // bIsPublic (private)
        OnResult                 // 콜백
    );
    
    PRINTLOG(TEXT("✅ API call sent to save preset"));
}

void UMVE_AUD_CustomizationManager::HandleSavePresetComplete(bool bSuccess, const FSavePresetResponseData& Data,
	const FString& ErrorCode)
{
	if (bSuccess)
	{
		PRINTLOG(TEXT("✅ Preset saved successfully to server"));
		PRINTLOG(TEXT("   Preset Description: %s"), *Data.Description);
		PRINTLOG(TEXT("   Preset Name: %s"), *Data.PresetName);
	}
	else
	{
		PRINTLOG(TEXT("❌ Failed to save preset: %s"), *ErrorCode);
	}
}

void UMVE_AUD_CustomizationManager::LoadAccessoryPresetFromServer()
{
	PRINTLOG(TEXT("=== LoadAccessoryPresetFromServer ==="));

	// GetPresetList API 호출 (includePublic = false, 자신의 프리셋만)
	FOnGetPresetListComplete OnResult;
	OnResult.BindUObject(this, &UMVE_AUD_CustomizationManager::HandleLoadPresetComplete);

	UMVE_API_Helper::GetPresetList(false, OnResult);
	PRINTLOG(TEXT("✅ Preset list request sent"));
}

void UMVE_AUD_CustomizationManager::HandleLoadPresetComplete(bool bSuccess, const FGetPresetListResponseData& Data,
	const FString& ErrorCode)
{
	PRINTLOG(TEXT("=== HandleLoadPresetComplete ==="));

	if (!bSuccess)
	{
		PRINTLOG(TEXT("❌ Failed to load preset list: %s"), *ErrorCode);
		return;
	}

	if (Data.Presets.Num() == 0)
	{
		PRINTLOG(TEXT("⚠️ No presets found for this user"));
		return;
	}

	// 첫 번째 프리셋 사용 (가장 최근 저장된 것)
	const FAccessoryPreset& FirstPreset = Data.Presets[0];
	PRINTLOG(TEXT("✅ Preset loaded from server"));
	PRINTLOG(TEXT("   Preset ID: %d"), FirstPreset.Id);
	PRINTLOG(TEXT("   Preset Name: %s"), *FirstPreset.PresetName);
	PRINTLOG(TEXT("   Created At: %s"), *FirstPreset.CreatedAt);

	// Accessories 배열 파싱
	if (FirstPreset.Accessories.Num() == 0)
	{
		PRINTLOG(TEXT("⚠️ Preset has no accessories"));
		return;
	}

	// 첫 번째 액세서리 데이터 추출
	const FAccessory& FirstAccessory = FirstPreset.Accessories[0];

	// FCustomizationData로 변환
	SavedCustomization.SocketName = FirstAccessory.SocketName;
	SavedCustomization.RelativeLocation = FirstAccessory.RelativeLocation;
	SavedCustomization.RelativeRotation = FirstAccessory.RelativeRotation;
	SavedCustomization.RelativeScale = FirstAccessory.RelativeScale;
	SavedCustomization.ModelUrl = FirstAccessory.ModelUrl;

	PRINTLOG(TEXT("✅ Customization data loaded:"));
	PRINTLOG(TEXT("   Model URL: %s"), *SavedCustomization.ModelUrl);
	PRINTLOG(TEXT("   Socket: %s"), *SavedCustomization.SocketName);
	PRINTLOG(TEXT("   Location: %s"), *SavedCustomization.RelativeLocation.ToString());
	PRINTLOG(TEXT("   Rotation: %s"), *SavedCustomization.RelativeRotation.ToString());
	PRINTLOG(TEXT("   Scale: %.2f"), SavedCustomization.RelativeScale);

	// 델리게이트 호출 (PlayerController에서 사용)
	if (OnPresetLoadedDelegate.IsBound())
	{
		OnPresetLoadedDelegate.Execute(SavedCustomization);
	}
}

void UMVE_AUD_CustomizationManager::OnLoadPresetResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		PRINTLOG(TEXT("❌ Failed to load preset from server"));
		return;
	}
	
	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseContent = Response->GetContentAsString();
	
	PRINTLOG(TEXT("=== Load Preset Response ==="));
	PRINTLOG(TEXT("Response Code: %d"), ResponseCode);
	PRINTLOG(TEXT("Response Body: %s"), *ResponseContent);
	
	if (ResponseCode == 200)
	{
		// ⭐ 서버가 배열을 직접 반환하므로 바로 역직렬화
		// ResponseContent = [{"socketName":"head_socket",...}]
		FCustomizationData LoadedData = DeserializeCustomizationData(ResponseContent);
		
		// 데이터 검증
		if (LoadedData.ModelUrl.IsEmpty())
		{
			PRINTLOG(TEXT("⚠️ Loaded preset has no model URL (user might not have saved preset yet)"));
			return;
		}
		
		PRINTLOG(TEXT("✅ Preset loaded successfully"));
		PRINTLOG(TEXT("   Model URL: %s"), *LoadedData.ModelUrl);
		PRINTLOG(TEXT("   Socket: %s"), *LoadedData.SocketName);
		PRINTLOG(TEXT("   Location: %s"), *LoadedData.RelativeLocation.ToString());
		PRINTLOG(TEXT("   Rotation: %s"), *LoadedData.RelativeRotation.ToString());
		PRINTLOG(TEXT("   Scale: %.2f"), LoadedData.RelativeScale);
		
		// 델리게이트 호출 (PlayerController에서 사용)
		if (OnPresetLoadedDelegate.IsBound())
		{
			OnPresetLoadedDelegate.Execute(LoadedData);
		}
	}
	else if (ResponseCode == 404)
	{
		PRINTLOG(TEXT("⚠️ No preset found for this user"));
	}
	else
	{
		PRINTLOG(TEXT("❌ Server error: %d - %s"), ResponseCode, *ResponseContent);
	}
}

void UMVE_AUD_CustomizationManager::SetRemoteModelUrl(const FString& RemoteUrl)
{
	CurrentRemoteURL = RemoteUrl;
	PRINTLOG(TEXT("✅ Remote URL saved: %s"), *CurrentRemoteURL);
}

void UMVE_AUD_CustomizationManager::TestLoadLocalGLBWithFakeURL(const FString& LocalGLBPath, const FString& FakeRemoteURL, UMVE_AUD_WidgetClass_PreviewWidget* InPreviewWidget)
{
	PRINTLOG(TEXT("=== TestLoadLocalGLBWithFakeURL ==="));
	PRINTLOG(TEXT("Local GLB Path: %s"), *LocalGLBPath);
	PRINTLOG(TEXT("Fake Remote URL: %s"), *FakeRemoteURL);

	// 1. 로컬 GLB 파일 존재 확인
	if (!FPaths::FileExists(LocalGLBPath))
	{
		PRINTLOG(TEXT("❌ Local GLB file does not exist: %s"), *LocalGLBPath);
		return;
	}

	// 2. 가짜 RemoteURL 설정
	CurrentRemoteURL = FakeRemoteURL;
	PRINTLOG(TEXT("✅ Fake Remote URL set: %s"), *CurrentRemoteURL);

	// 3. GLB 파일 경로 저장
	CurrentGLBFilePath = LocalGLBPath;

	// 4. 메시 프리뷰 시작
	StartMeshPreview(LocalGLBPath, InPreviewWidget);

	PRINTLOG(TEXT("✅ Test mode: Local GLB loaded with fake remote URL"));
	PRINTLOG(TEXT("   Now you can:"));
	PRINTLOG(TEXT("   1. Attach mesh to socket (Head/LeftHand/RightHand buttons)"));
	PRINTLOG(TEXT("   2. Click Save button to test preset saving"));
}
