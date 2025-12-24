// // Fill out your copyright notice in the Description page of Project Settings.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "OSCServer.h"
// #include "Components/ActorComponent.h"
// #include "RemocapMotionReceiverComponent.generated.h"
//
// UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
// class MVE_API URemocapMotionReceiverComponent : public UActorComponent
// {
// 	GENERATED_BODY()
//
// public:
// 	// Sets default values for this component's properties
// 	URemocapMotionReceiverComponent();
//
// protected:
// 	// Called when the game starts
// 	virtual void BeginPlay() override;
// 	
// 	UPROPERTY(VisibleAnywhere)
// 	TObjectPtr<UOSCServer> OSCServer;
// 	
// 	UFUNCTION()
// 	void OnOnOscBundleReceived(const FOSCBundle& Bundle, const FString& IPAddress, int32 Port);
//
// 	UFUNCTION(BlueprintCallable, Category = "Remocap")
// 	bool RetrieveControlData(const FOSCMessage& Message, int32& ControlIndex, FTransform& ControlTransform);
// 	
// 	UFUNCTION()
// 	bool GetFloatSafe(const TArray<FOSCType>& FloatArray, int32 Index, float& OutValue);
// };
