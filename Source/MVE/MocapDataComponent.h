#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"
#include "CompressedMotionData.h"
#include "MocapDataComponent.generated.h"

// 일반 데이터
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlDataReceived, int32, ControlIndex, FTransform, ControlTransform);

// 압축 프레임
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompressedFrameReceived, FCompressedMotionFrame, CompressedFrame);

// 페이스 데이터 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaceMorphDataReceived, const TArray<float>&, MorphTargets);

UCLASS(ClassGroup=(Mocap), meta=(BlueprintSpawnableComponent))
class MVE_API UMocapDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMocapDataComponent();
	virtual ~UMocapDataComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Mocap")
	FOnControlDataReceived OnControlDataReceived;

	UPROPERTY(BlueprintAssignable, Category = "Mocap")
	FOnCompressedFrameReceived OnCompressedFrameReceived;

	UPROPERTY(BlueprintAssignable, Category = "Mocap")
	FOnFaceMorphDataReceived OnFaceMorphDataReceived;
	
	// OSC 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	int32 ReceivePort = 39570;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	FName AddressFilter = FName(TEXT("/Remocapp"));

	// 페이스 데이터 활성화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	bool bEnableFaceData = false;

	// 압축 옵션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
	bool bUseCompression = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
	bool bUseDeltaCompression = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
	float DeltaThreshold = 0.01f;

	// 디버깅
	UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
	int32 GetLastFrameBoneCount() const { return LastFrameBoneCount; }

	UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
	float GetCompressionRatio() const { return CompressionRatio; }
	
	// OSC Receiver
	class FMocapOscReceiver : public IOscReceiverInterface
	{
	public:
		UMocapDataComponent* Owner;

		FMocapOscReceiver(UMocapDataComponent* InOwner) : Owner(InOwner) {}

		virtual FName GetAddressFilter() const override
		{
			return Owner ? Owner->AddressFilter : FName();
		}

		virtual void SendEvent(const FName& Address, const TArray<FOscDataElemStruct>& Data, const FString& SenderIp) override
		{
			if (Owner)
			{
				Owner->OnOscMessageReceived(Address, Data, SenderIp);
			}
		}
	};

	TUniquePtr<FMocapOscReceiver> OscReceiver;

	// OSC 메시지 수신
	void OnOscMessageReceived(const FName& Address, const TArray<FOscDataElemStruct>& Data, const FString& SenderIp);
	
	// 데이터 처리
	void ProcessControlData(const TArray<FOscDataElemStruct>& Data);
	void ProcessFaceMorphData(const TArray<FOscDataElemStruct>& Data);

	// 압축 관련
	struct FBoneData
	{
		int32 Index;
		FVector Location;
		FQuat Rotation;
	};

	TArray<FBoneData> CurrentFrameBones;
	float FrameStartTime = 0.0f;
	bool bFrameStarted = false;

	FCompressedMotionFrame CurrentCompressedFrame;
	TMap<uint8, FCompressedBoneTransform> BoneCache;

	void ProcessCompressedFrame();
	bool ShouldSendBone(uint8 BoneIndex, const FCompressedBoneTransform& NewTransform);

	// 통계
	int32 LastFrameBoneCount = 0;
	float CompressionRatio = 0.0f;
};