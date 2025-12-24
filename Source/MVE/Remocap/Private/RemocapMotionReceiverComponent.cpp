// // Fill out your copyright notice in the Description page of Project Settings.
//
// #include "Remocap/Public/RemocapMotionReceiverComponent.h"
//
// #include "MVE.h"
// #include "OSCServer.h"
// #include "OSCAddress.h"
// #include "OSCMessage.h"
// #include "OSCManager.h"
//
// URemocapMotionReceiverComponent::URemocapMotionReceiverComponent()
// {
// 	// 틱 비활성화
// 	PrimaryComponentTick.bCanEverTick = false;
// }
//
// void URemocapMotionReceiverComponent::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	// 서버 생성
// 	OSCServer = NewObject<UOSCServer>(this);
// 	if (OSCServer)
// 	{
// 		OSCServer->SetAddress({"0.0.0.0"}, {39570});
// 		OSCServer->OnOscBundleReceived.AddDynamic(this, &URemocapMotionReceiverComponent::OnOnOscBundleReceived);
// 	}
// }
//
// // ReSharper disable once CppMemberFunctionMayBeConst
// void URemocapMotionReceiverComponent::OnOnOscBundleReceived(const FOSCBundle& Bundle, const FString& IPAddress, int32 Port)
// {
// 	// 번들로부터 0번 인덱스에 있는 메시지를 추출 시도
// 	bool bSucceeded;
// 	const FOSCMessage OSCMessages = UOSCManager::GetMessageFromBundle(Bundle, 0, bSucceeded);
// 	if (bSucceeded)
// 	{
// 		for (auto OSCMessage : OSCMessages)
// 		{
// 			const FOSCAddress OSCAddress = UOSCManager::GetOSCMessageAddress(OSCMessage);
// 			switch (const FString OSCAddressFullPath = OSCAddress.GetFullPath())
// 			{
// 			case TEXT("/Remocapp/UE"):
// 				{
// 					int32 ControlIndex;
// 					FTransform ControlTransform;
// 					if (RetrieveControlData(OSCMessage, ControlIndex, ControlTransform))
// 					{
// 						// TODO: ControlIndex와 ControlTransform을 사용하는 로직 추가
// 					}
// 					break;
// 				}
// 			case TEXT("/Remocapp/UE/Face"):
// 				{
// 					break;
// 				}
// 			default:
// 				{
// 					break;
// 				}
// 			}
// 		}
// 	}
// }
//
// bool URemocapMotionReceiverComponent::RetrieveControlData(const FOSCMessage& Message, int32& ControlIndex, FTransform& ControlTransform)
// {
// 	const TArray<FOSCType>& Args = Message.GetArguments();
//     
// 	// 최소 8개 인자 필요: 1개 int + 7개 float
// 	if (Args.Num() < 8)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("OSC 메시지 인자 부족: %d개 (최소 8개 필요)"), Args.Num());
// 		return false;
// 	}
//     
// 	// [0] Control Index 추출
// 	if (!Args[0].GetInt32(ControlIndex))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Control Index 파싱 실패"));
// 		return false;
// 	}
//     
// 	// Float 배열에서 Transform 데이터 추출
// 	float PosX = 0.f, PosY = 0.f, PosZ = 0.f;
// 	float RotX = 0.f, RotY = 0.f, RotZ = 0.f, RotW = 1.f;
//     
// 	// Position [1-3]
// 	if (!GetFloatSafe(Args, 1, PosX) ||
// 		!GetFloatSafe(Args, 2, PosY) ||
// 		!GetFloatSafe(Args, 3, PosZ))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Position 데이터 파싱 실패 (Control Index: %d)"), 
// 			   ControlIndex);
// 		return false;
// 	}
//     
// 	// Rotation [4-7]
// 	if (!GetFloatSafe(Args, 4, RotX) ||
// 		!GetFloatSafe(Args, 5, RotY) ||
// 		!GetFloatSafe(Args, 6, RotZ) ||
// 		!GetFloatSafe(Args, 7, RotW))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Rotation 데이터 파싱 실패 (Control Index: %d)"), 
// 			   ControlIndex);
// 		return false;
// 	}
//     
// 	// Transform 생성
// 	const FVector Location(PosX, PosY, PosZ);
// 	FQuat Rotation(RotX, RotY, RotZ, RotW);
//     
// 	// Quaternion 정규화 (네트워크 전송 오류 대비)
// 	if (Rotation.IsNormalized() == false)
// 	{
// 		Rotation.Normalize();
// 		UE_LOG(LogTemp, Warning, TEXT("Quaternion 정규화 수행 (Control Index: %d)"), ControlIndex);
// 	}
//     
// 	ControlTransform = FTransform(Rotation, Location, FVector::OneVector);
// 	return true;
// }
//
// bool URemocapMotionReceiverComponent::GetFloatSafe(const TArray<FOSCType>& FloatArray, const int32 Index, float& OutValue)
// {
// 	if (FloatArray.IsValidIndex(Index) == false)
// 	{
// 		return false;
// 	}
//     
// 	OutValue = FloatArray[Index].GetFloat(); 
// 	return true;	
// }