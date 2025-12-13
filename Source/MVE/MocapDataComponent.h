#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Receive/OscReceiverInterface.h"
#include "Common/OscDataElemStruct.h"
#include "MocapDataComponent.generated.h"

//컨트롤 데이터 수신 (Index, Transform)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlDataReceived, int32, ControlIndex, FTransform, ControlTransform);

//페이스 모프 데이터 수신 (수신만 받는용도)
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

	// 블루프린트에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Mocap")
	FOnControlDataReceived OnControlDataReceived;

	UPROPERTY(BlueprintAssignable, Category = "Mocap")
	FOnFaceMorphDataReceived OnFaceMorphDataReceived;

	// OSC 수신 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	int32 ReceivePort = 39570;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	FName AddressFilter = FName(TEXT("/Remocapp"));
	
	// OSC Receiver 구현
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

	void OnOscMessageReceived(const FName& Address, const TArray<FOscDataElemStruct>& Data, const FString& SenderIp);
	void ProcessControlData(const TArray<FOscDataElemStruct>& Data);
	void ProcessFaceMorphData(const TArray<FOscDataElemStruct>& Data);


	// 페이스 데이터 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
	bool bEnableFaceData = false;
};