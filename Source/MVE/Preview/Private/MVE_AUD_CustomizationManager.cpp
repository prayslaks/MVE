#include "../Public/MVE_AUD_CustomizationManager.h"
#include "DesktopPlatformModule.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeParser.h"
#include "HttpModule.h"
#include "IDesktopPlatform.h"
#include "MVE.h"
#include "MVE_AUD_PreviewCameraPawn.h"
#include "MVE_AUD_PreviewCaptureActor.h"
#include "MVE_AUD_WidgetClass_PreviewWidget.h"
#include "MVE_GM_PreviewMesh.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/TextureRenderTarget2D.h"


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

void UMVE_AUD_CustomizationManager::RequestModelGeneration(const FString& PromptText)
{
	
	if (ReferenceImageData.Num() == 0)
	{
		PRINTLOG(TEXT("No reference image attached"));
		return;
	}
	

	// 요청 데이터 생성
	FInputPromptData Request;
	Request.PromptMessageText = PromptText;
	Request.ReferenceImageData = ReferenceImageData;
	Request.ImageFormat = ReferenceImageFormat;
	Request.UserID = GetWorld()->GetFirstPlayerController()->GetUniqueID();

	
	SendToExternalServer(Request);

	// 임시 데이터 클리어
	ReferenceImageData.Empty();
	ReferenceImageFormat.Empty();
}

bool UMVE_AUD_CustomizationManager::LoadReferenceImage(const FString& FilePath)
{
	// 기존 데이터 클리어
	ReferenceImageData.Empty();
	ReferenceImageFileName.Empty();
	ReferenceImageFormat.Empty();

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
		return false;
	}

	return true;
}

void UMVE_AUD_CustomizationManager::SendToExternalServer(const FInputPromptData& Request)
{
	PRINTLOG(TEXT("=== Sending to AI Server ==="));
    PRINTLOG(TEXT("Prompt: %s"), *Request.PromptMessageText);
    PRINTLOG(TEXT("Image Format: %s"), *Request.ImageFormat);
    PRINTLOG(TEXT("Image Size: %d bytes"), Request.ReferenceImageData.Num());

    // HTTP 모듈 가져오기
    FHttpModule* HttpModule = &FHttpModule::Get();
    TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

    // 요청 설정
    HttpRequest->SetURL(TEXT("http://172.16.20.234:8001/generate_mesh"));
    HttpRequest->SetVerb(TEXT("POST"));

    // Boundary 생성
    FString Boundary = FString::Printf(TEXT("----UnrealBoundary%d"), FDateTime::Now().GetTicks());
    HttpRequest->SetHeader(TEXT("Content-Type"), 
        FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    PRINTLOG(TEXT("Boundary: %s"), *Boundary);

    TArray<uint8> BodyData;

    // 1. JSON metadata 파트
    {
        // JSON 객체 생성
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField(TEXT("prompt"), Request.PromptMessageText);
        JsonObject->SetStringField(TEXT("user_email"), TEXT("test_user@example.com"));
        JsonObject->SetStringField(TEXT("request_id"), FGuid::NewGuid().ToString());

        // JSON 직렬화
        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

        PRINTLOG(TEXT("JSON Metadata: %s"), *JsonString);

        // Multipart 헤더 + JSON 본문
        FString Part;
        Part += FString::Printf(TEXT("--%s\r\n"), *Boundary);
        Part += TEXT("Content-Disposition: form-data; name=\"metadata\"\r\n");
        Part += TEXT("Content-Type: application/json\r\n\r\n");
        Part += JsonString;
        Part += TEXT("\r\n");

        // UTF-8로 변환하여 추가
        FTCHARToUTF8 Converter(*Part);
        BodyData.Append((const uint8*)Converter.Get(), Converter.Length());
    }

    // 2. 이미지 파일 파트
    {
        // MIME 타입 결정
        FString MimeType = TEXT("image/png");
        if (Request.ImageFormat == TEXT("png"))
            MimeType = TEXT("image/png");
        else if (Request.ImageFormat == TEXT("jpg") || Request.ImageFormat == TEXT("jpeg"))
            MimeType = TEXT("image/jpeg");
        else if (Request.ImageFormat == TEXT("gif"))
            MimeType = TEXT("image/gif");
        else if (Request.ImageFormat == TEXT("webp"))
            MimeType = TEXT("image/webp");

        // Multipart 헤더
        FString Header;
        Header += FString::Printf(TEXT("--%s\r\n"), *Boundary);
        Header += FString::Printf(TEXT("Content-Disposition: form-data; name=\"image\"; filename=\"reference.%s\"\r\n"),
            *Request.ImageFormat);
        Header += FString::Printf(TEXT("Content-Type: %s\r\n\r\n"), *MimeType);

        FTCHARToUTF8 HeaderConv(*Header);
        BodyData.Append((uint8*)HeaderConv.Get(), HeaderConv.Length());

        // 이미지 바이너리 데이터 추가
        BodyData.Append(Request.ReferenceImageData);

        // 줄바꿈 추가
        FString LineBreak = TEXT("\r\n");
        FTCHARToUTF8 LBConv(*LineBreak);
        BodyData.Append((uint8*)LBConv.Get(), LBConv.Length());
    }

    // 3. 종료 boundary
    {
        FString Closing = FString::Printf(TEXT("--%s--\r\n"), *Boundary);
        FTCHARToUTF8 Converter(*Closing);
        BodyData.Append((const uint8*)Converter.Get(), Converter.Length());
    }

    PRINTLOG(TEXT("Total Body Size: %d bytes"), BodyData.Num());

    // HTTP 요청에 본문 설정
    HttpRequest->SetContent(BodyData);

    // 응답 콜백 바인딩
    HttpRequest->OnProcessRequestComplete().BindUObject(
        this, &UMVE_AUD_CustomizationManager::OnModelGenerationResponse);

    // 요청 전송
    if (HttpRequest->ProcessRequest())
    {
        PRINTLOG(TEXT("✅ Model generation request sent to AI server"));
    }
    else
    {
        PRINTLOG(TEXT("❌ Failed to send HTTP request"));
    }
}

void UMVE_AUD_CustomizationManager::OnModelGenerationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		PRINTLOG(TEXT("❌ Failed to connect to AI server"));
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseContent = Response->GetContentAsString();
    
	PRINTLOG(TEXT("=== AI Server Response ==="));
	PRINTLOG(TEXT("Response Code: %d"), ResponseCode);
	PRINTLOG(TEXT("Response Body: %s"), *ResponseContent);

	if (ResponseCode == 200)
	{
		// JSON 응답 파싱
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString ModelID = JsonObject->GetStringField(TEXT("model_id"));
			FString GLBFileURL = JsonObject->GetStringField(TEXT("glb_url"));
            
			PRINTLOG(TEXT("✅ Model generation queued: %s"), *ModelID);
			PRINTLOG(TEXT("✅ GLB URL: %s"), *GLBFileURL);
            
			// 생성 완료 처리
			OnModelGenerationComplete(ModelID, GLBFileURL);
		}
		else
		{
			PRINTLOG(TEXT("❌ Failed to parse JSON response"));
		}
	}
	else if (ResponseCode == 422)
	{
		PRINTLOG(TEXT("❌ Validation Error (422): %s"), *ResponseContent);
        
		// JSON 파싱해서 상세 에러 확인
		TSharedPtr<FJsonObject> ErrorJson;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
		if (FJsonSerializer::Deserialize(Reader, ErrorJson))
		{
			FString ErrorMsg = ErrorJson->GetStringField(TEXT("error"));
			PRINTLOG(TEXT("Server Error Message: %s"), *ErrorMsg);
		}
	}
	else
	{
		PRINTLOG(TEXT("❌ AI server error: %d - %s"), ResponseCode, *ResponseContent);
	}
}

void UMVE_AUD_CustomizationManager::OnModelGenerationComplete(const FString& ModelID, const FString& GLBFileURL)
{
	PRINTLOG(TEXT("Model generation complete: %s"), *ModelID);

	// GLB 파일 다운로드 및 로딩
	// glTFRuntime으로 런타임 로딩
	// 기존 캐릭터에 악세서리 부착
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
        PRINTLOG(TEXT("   Final Scale: %s"), *NewMesh->GetComponentScale().ToString());
        
        FVector FinalOrigin, FinalExtent;
        NewAccessory->GetActorBounds(false, FinalOrigin, FinalExtent);
        PRINTLOG(TEXT("   Final Size: X=%.2f, Y=%.2f, Z=%.2f cm"), 
            FinalExtent.X * 2.0f, FinalExtent.Y * 2.0f, FinalExtent.Z * 2.0f);
    }
    else
    {
        PRINTLOG(TEXT("⚠️ Socket not found: %s"), *SocketName.ToString());
        NewAccessory->Destroy();
        return;
    }

    AttachedMesh = NewAccessory;

    // 커스터마이징 데이터 저장
    SavedCustomization.GLBFilePath = CurrentGLBFilePath;
    SavedCustomization.SocketName = SocketName;
    SavedCustomization.RelativeTransform = NewAccessory->GetTransform().GetRelativeTransform(SkelMesh->GetComponentTransform());

    PRINTLOG(TEXT("✅ Customization data saved:"));
    PRINTLOG(TEXT("   GLB Path: %s"), *SavedCustomization.GLBFilePath);
    PRINTLOG(TEXT("   Socket: %s"), *SavedCustomization.SocketName.ToString());
    PRINTLOG(TEXT("   Transform: %s"), *SavedCustomization.RelativeTransform.ToString());

    // ⭐ 자동으로 기즈모 모드 전환 (옵션 B: CameraPawn에서 처리)
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