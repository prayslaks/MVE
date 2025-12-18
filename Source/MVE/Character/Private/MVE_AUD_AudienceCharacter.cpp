
#include "../Public/MVE_AUD_AudienceCharacter.h"

#include "MVE.h"
#include "Camera/CameraComponent.h"
#include "Character/Public/MVE_AUD_InteractionComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Preview/Public/MVE_AUD_CustomizationManager.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeParser.h"

AMVE_AUD_AudienceCharacter::AMVE_AUD_AudienceCharacter()
{
	InteractionComponent = CreateDefaultSubobject<UMVE_AUD_InteractionComponent>(TEXT("InteractionComponent"));
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeLocation(FVector(0.f));
}

void AMVE_AUD_AudienceCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 커스터마이징 적용
	if (IsLocallyControlled())
	{
		ApplyCustomization();
	}
}

void AMVE_AUD_AudienceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMVE_AUD_AudienceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMVE_AUD_AudienceCharacter::ApplyCustomization()
{
	/*
	PRINTLOG(TEXT("=== ApplyCustomization called ==="));

	// CustomizationManager 가져오기
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		PRINTLOG(TEXT("❌ GameInstance is null"));
		return;
	}

	UMVE_AUD_CustomizationManager* CustomizationManager = GI->GetSubsystem<UMVE_AUD_CustomizationManager>();
	if (!CustomizationManager)
	{
		PRINTLOG(TEXT("❌ CustomizationManager is null"));
		return;
	}

	// 저장된 커스터마이징 데이터 가져오기
	FCustomizationData SavedData = CustomizationManager->GetSavedCustomization();

	// 데이터 유효성 확인
	if (SavedData.ModelUrl.IsEmpty() || SavedData.SocketName == NAME_None)
	{
		PRINTLOG(TEXT("⚠️ No customization data saved"));
		return;
	}

	PRINTLOG(TEXT("✅ Loading customization:"));
	PRINTLOG(TEXT("   GLB Path: %s"), *SavedData.GLBFilePath);
	PRINTLOG(TEXT("   Socket: %s"), *SavedData.SocketName.ToString());
	PRINTLOG(TEXT("   Transform: %s"), *SavedData.RelativeTransform.ToString());

	// GLB 파일 존재 확인
	if (!FPaths::FileExists(SavedData.GLBFilePath))
	{
		PRINTLOG(TEXT("❌ GLB file does not exist: %s"), *SavedData.GLBFilePath);
		return;
	}

	// GLB 파일 로딩
	FglTFRuntimeConfig LoaderConfig;
	LoaderConfig.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;

	UglTFRuntimeAsset* Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
		SavedData.GLBFilePath,
		false,
		LoaderConfig
	);

	if (!Asset)
	{
		PRINTLOG(TEXT("❌ Failed to load GLB file: %s"), *SavedData.GLBFilePath);
		return;
	}

	PRINTLOG(TEXT("✅ GLB asset loaded"));

	// Static Mesh 생성
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;
	UStaticMesh* StaticMesh = Asset->LoadStaticMesh(0, StaticMeshConfig);

	if (!StaticMesh)
	{
		PRINTLOG(TEXT("❌ Failed to load static mesh from GLB"));
		return;
	}

	PRINTLOG(TEXT("✅ Static mesh loaded"));

	// 액세서리 액터 생성
	AActor* MeshActor = GetWorld()->SpawnActor<AActor>();
	if (!MeshActor)
	{
		PRINTLOG(TEXT("❌ Failed to spawn mesh actor"));
		return;
	}

	// Static Mesh Component 추가
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(MeshActor);
	if (!MeshComponent)
	{
		PRINTLOG(TEXT("❌ Failed to create mesh component"));
		MeshActor->Destroy();
		return;
	}

	MeshComponent->SetStaticMesh(StaticMesh);
	MeshActor->SetRootComponent(MeshComponent);
	MeshComponent->RegisterComponent();

	PRINTLOG(TEXT("✅ Mesh component created"));

	// 콜백 호출
	OnCustomizationMeshLoaded(MeshActor, SavedData.SocketName, SavedData.RelativeTransform);
	*/
}

void AMVE_AUD_AudienceCharacter::OnCustomizationMeshLoaded(AActor* LoadedActor, FName SocketName, FTransform RelativeTransform)
{
	PRINTLOG(TEXT("=== OnCustomizationMeshLoaded called ==="));

	if (!LoadedActor)
	{
		PRINTLOG(TEXT("❌ LoadedActor is null"));
		return;
	}

	// 캐릭터의 Skeletal Mesh 가져오기
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (!SkelMesh)
	{
		PRINTLOG(TEXT("❌ No skeletal mesh found on character"));
		LoadedActor->Destroy();
		return;
	}

	// 소켓 존재 확인
	if (!SkelMesh->DoesSocketExist(SocketName))
	{
		PRINTLOG(TEXT("❌ Socket not found: %s"), *SocketName.ToString());
		LoadedActor->Destroy();
		return;
	}

	// 소켓에 부착
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false
	);

	LoadedActor->AttachToComponent(SkelMesh, AttachRules, SocketName);

	// 저장된 Transform 적용
	LoadedActor->SetActorRelativeTransform(RelativeTransform);

	PRINTLOG(TEXT("✅ Customization applied successfully!"));
	PRINTLOG(TEXT("   Attached to socket: %s"), *SocketName.ToString());
	PRINTLOG(TEXT("   Transform: %s"), *RelativeTransform.ToString());
}