
#include "MVE_GS_StageLevel.h"
#include "MVE.h"
#include "Net/UnrealNetwork.h"
#include "SenderReceiver.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "MVE_AUD_CustomizationManager.h"

AMVE_GS_StageLevel::AMVE_GS_StageLevel()
{
	ViewerCount = 0;
	bReplicates = true;
}

void AMVE_GS_StageLevel::SetViewerCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		PRINTLOG(TEXT("SetViewerCount can only be called on server!"));
		return;
	}

	if (ViewerCount != NewCount)
	{
		ViewerCount = NewCount;
		PRINTLOG(TEXT("ViewerCount updated to: %d"), ViewerCount);

		// Broadcast on server immediately
		OnViewerCountChanged.Broadcast(ViewerCount);
	}
}

void AMVE_GS_StageLevel::OnRep_ViewerCount()
{
	PRINTLOG(TEXT("OnRep_ViewerCount: %d"), ViewerCount);

	// Broadcast to widgets on clients
	OnViewerCountChanged.Broadcast(ViewerCount);
}

void AMVE_GS_StageLevel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMVE_GS_StageLevel, ViewerCount);
}

void AMVE_GS_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// SenderReceiver의 OnAssetLoaded 델리게이트에 바인딩
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USenderReceiver* SR = GI->GetSubsystem<USenderReceiver>())
		{
			SR->OnAssetLoaded.AddDynamic(this, &AMVE_GS_StageLevel::OnAccessoryLoaded);
			PRINTLOG(TEXT("✅ GameState bound to SenderReceiver::OnAssetLoaded"));
		}
	}
}

void AMVE_GS_StageLevel::MulticastRPC_BroadcastAccessory_Implementation(const FString& UserID, const FString& PresetJSON)
{
	PRINTLOG(TEXT("=== MulticastRPC_BroadcastAccessory (GameState) ==="));
	PRINTLOG(TEXT("UserID: %s"), *UserID);
	PRINTLOG(TEXT("PresetJSON: %s"), *PresetJSON);

	// CustomizationManager 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager =
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (!CustomizationManager)
	{
		PRINTLOG(TEXT("❌ CustomizationManager not found"));
		return;
	}

	// JSON 역직렬화
	FCustomizationData Data = CustomizationManager->DeserializeCustomizationData(PresetJSON);

	// 데이터 검증
	if (Data.ModelUrl.IsEmpty())
	{
		PRINTLOG(TEXT("⚠️ ModelUrl is empty for UserID: %s"), *UserID);
		return;
	}

	PRINTLOG(TEXT("✅ Deserialized data:"));
	PRINTLOG(TEXT("   Model URL: %s"), *Data.ModelUrl);
	PRINTLOG(TEXT("   Socket: %s"), *Data.SocketName);
	PRINTLOG(TEXT("   Location: %s"), *Data.RelativeLocation.ToString());

	// 대기 맵에 저장 (다운로드 완료 후 매칭용)
	PendingAccessories.Add(UserID, Data);

	// SenderReceiver 사용
	USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
	if (!SR)
	{
		PRINTLOG(TEXT("❌ SenderReceiver not found"));
		return;
	}

	// 메타데이터 구성
	FAssetMetadata Metadata;
	Metadata.AssetType = EAssetType::MESH;
	Metadata.UserEmail = UserID;  // ⭐ 매칭용 키
	Metadata.RemotePath = Data.ModelUrl;  // PresignedURL
	Metadata.AssetID = FGuid::NewGuid();

	PRINTLOG(TEXT("✅ Queuing download:"));
	PRINTLOG(TEXT("   AssetID: %s"), *Metadata.AssetID.ToString());
	PRINTLOG(TEXT("   UserID: %s"), *UserID);
	PRINTLOG(TEXT("   RemotePath: %s"), *Metadata.RemotePath);

	// 다운로드 시작
	SR->DownloadFromFileServer(Metadata);

	PRINTLOG(TEXT("✅ Download queued for UserID: %s"), *UserID);
}

APawn* AMVE_GS_StageLevel::FindCharacterByUserID(const FString& UserID) const
{
	PRINTLOG(TEXT("=== FindCharacterByUserID ==="));
	PRINTLOG(TEXT("Searching for UserID: %s"), *UserID);
	PRINTLOG(TEXT("PlayerArray count: %d"), PlayerArray.Num());

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (!PlayerState) continue;

		FString PlayerName = PlayerState->GetPlayerName();
		PRINTLOG(TEXT("  Checking PlayerState: %s"), *PlayerName);

		if (PlayerName.Equals(UserID, ESearchCase::IgnoreCase))
		{
			APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner());
			if (PC)
			{
				APawn* Pawn = PC->GetPawn();
				PRINTLOG(TEXT("✅ Found character for UserID: %s"), *UserID);
				return Pawn;
			}
		}
	}

	PRINTLOG(TEXT("❌ Character not found for UserID: %s"), *UserID);
	return nullptr;
}

void AMVE_GS_StageLevel::OnAccessoryLoaded(UObject* Asset, const FAssetMetadata& Metadata)
{
	PRINTLOG(TEXT("=== OnAccessoryLoaded (GameState) ==="));
	PRINTLOG(TEXT("DisplayName: %s"), *Metadata.DisplayName);
	PRINTLOG(TEXT("AssetID: %s"), *Metadata.AssetID.ToString());

	FString UserID = Metadata.UserEmail;

	FCustomizationData* Data = PendingAccessories.Find(UserID);
	if (!Data)
	{
		PRINTLOG(TEXT("⚠️ No pending accessory for UserID: %s"), *UserID);
		return;
	}

	if (!Asset)
	{
		PRINTLOG(TEXT("❌ Asset is null"));
		PendingAccessories.Remove(UserID);
		return;
	}

	PRINTLOG(TEXT("✅ Asset loaded successfully"));
	PRINTLOG(TEXT("   Asset Type: %s"), *Asset->GetClass()->GetName());
	PRINTLOG(TEXT("   Socket: %s"), *Data->SocketName);

	// 액세서리 적용
	ApplyAccessoryToCharacter(UserID, Asset, *Data);

	// 정리
	PendingAccessories.Remove(UserID);
}

void AMVE_GS_StageLevel::ApplyAccessoryToCharacter(const FString& UserID, UObject* Asset, const FCustomizationData& Data)
{
	PRINTLOG(TEXT("=== ApplyAccessoryToCharacter (GameState) ==="));
	PRINTLOG(TEXT("UserID: %s"), *UserID);
	PRINTLOG(TEXT("Socket: %s"), *Data.SocketName);

	// 1. 캐릭터 찾기
	APawn* Character = FindCharacterByUserID(UserID);
	if (!Character)
	{
		PRINTLOG(TEXT("❌ Character not found for UserID: %s"), *UserID);
		return;
	}

	PRINTLOG(TEXT("✅ Character found: %s"), *Character->GetName());

	// ⭐ THROW_MESH 특수 처리 (소켓에 부착하지 않고 GameState에 UserID별로 저장)
	if (Data.SocketName == TEXT("THROW_MESH"))
	{
		PRINTLOG(TEXT("✅ THROW_MESH detected - storing in GameState by UserID"));

		UStaticMesh* ThrowMesh = Cast<UStaticMesh>(Asset);
		if (ThrowMesh)
		{
			// ⭐ GameState에 UserID별로 저장
			UserThrowMeshes.Add(UserID, ThrowMesh);
			PRINTLOG(TEXT("✅ Throw mesh stored for UserID: %s"), *UserID);

			// ⭐ 로컬 플레이어의 CustomizationManager에도 저장 (자신이 던질 때 사용)
			if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
			{
				if (UMVE_AUD_CustomizationManager* CustomizationManager = GameInstance->GetSubsystem<UMVE_AUD_CustomizationManager>())
				{
					CustomizationManager->SetThrowMeshDirect(ThrowMesh);
				}
			}
		}
		return;  // 소켓 부착하지 않고 종료
	}

	// 2. Skeletal Mesh Component 가져오기 (일반 액세서리용)
	USkeletalMeshComponent* SkelMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
	if (!SkelMesh)
	{
		PRINTLOG(TEXT("❌ No skeletal mesh component found on character"));
		return;
	}

	// 3. Socket 존재 확인 (일반 액세서리용)
	FName SocketName = FName(*Data.SocketName);
	if (!SkelMesh->DoesSocketExist(SocketName))
	{
		PRINTLOG(TEXT("❌ Socket '%s' does not exist on character"), *Data.SocketName);
		return;
	}

	PRINTLOG(TEXT("✅ Socket '%s' found"), *Data.SocketName);

	// 4. StaticMesh 추출 (SenderReceiver는 UStaticMesh 또는 USkeletalMesh를 반환)
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset);

	if (!StaticMesh)
	{
		PRINTLOG(TEXT("⚠️ Asset is not StaticMesh, trying SkeletalMesh fallback..."));

		// SkeletalMesh인 경우는 현재 지원하지 않음 (필요시 추가 구현)
		USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset);
		if (SkeletalMesh)
		{
			PRINTLOG(TEXT("❌ SkeletalMesh accessories not supported yet"));
			return;
		}

		PRINTLOG(TEXT("❌ Asset is neither StaticMesh nor SkeletalMesh"));
		return;
	}

	PRINTLOG(TEXT("✅ StaticMesh extracted successfully"));

	// 5. 액세서리 액터 생성
	AActor* AccessoryActor = GetWorld()->SpawnActor<AActor>();
	if (!AccessoryActor)
	{
		PRINTLOG(TEXT("❌ Failed to spawn accessory actor"));
		return;
	}

	// 6. StaticMeshComponent 생성
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(AccessoryActor);
	if (!MeshComponent)
	{
		PRINTLOG(TEXT("❌ Failed to create mesh component"));
		AccessoryActor->Destroy();
		return;
	}

	MeshComponent->SetStaticMesh(StaticMesh);
	AccessoryActor->SetRootComponent(MeshComponent);
	MeshComponent->RegisterComponent();

	PRINTLOG(TEXT("✅ Mesh component created and registered"));

	// 7. 먼저 KeepWorld로 부착 (현재 월드 트랜스폼 유지)
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepWorld,  // Location - 월드 트랜스폼 유지
		EAttachmentRule::KeepWorld,  // Rotation - 월드 트랜스폼 유지
		EAttachmentRule::KeepWorld,  // Scale - 월드 트랜스폼 유지
		false
	);

	AccessoryActor->AttachToComponent(SkelMesh, AttachRules, SocketName);

	PRINTLOG(TEXT("✅ Accessory attached to socket with KeepWorld: %s"), *Data.SocketName);

	// 8. 부착된 상태에서 Relative Transform 설정
	FTransform RelativeTransform;
	RelativeTransform.SetLocation(Data.RelativeLocation);
	RelativeTransform.SetRotation(Data.RelativeRotation.Quaternion());
	RelativeTransform.SetScale3D(FVector(Data.RelativeScale));

	AccessoryActor->SetActorRelativeTransform(RelativeTransform);

	PRINTLOG(TEXT("✅ Relative Transform applied:"));
	PRINTLOG(TEXT("   Location: %s"), *Data.RelativeLocation.ToString());
	PRINTLOG(TEXT("   Rotation: %s"), *Data.RelativeRotation.ToString());
	PRINTLOG(TEXT("   Scale: %.2f"), Data.RelativeScale);

	PRINTLOG(TEXT("✅ Accessory successfully applied to character!"));
}

UStaticMesh* AMVE_GS_StageLevel::GetThrowMeshForUser(const FString& UserID) const
{
	// FindRef는 키가 없으면 nullptr 반환
	return UserThrowMeshes.FindRef(UserID);
}
