// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_PC_StageLevel_AudienceComponent.generated.h"

class USoundBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_PC_StageLevel_AudienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_PC_StageLevel_AudienceComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 응원봉 흔들기 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> SwingStickSound;

	// 사진 찍기 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> TakePhotoSound;

	// 환호 사운드
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Sound")
	TObjectPtr<USoundBase> CheerUpSound;

public:
	// 서버에게 응원봉 흔들기를 요청
	UFUNCTION(Server, Reliable)
	void SwingLightStickOnServer(const FVector& Location);

	// 모든 클라이언트에게 응원봉 흔들기 이펙트를 동기화
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwingLightStick(const FVector& Location);

	// 서버에게 사진 찍기를 요청
	UFUNCTION(Server, Reliable)
	void TakePhotoOnServer(const FVector& Location);

	// 모든 클라이언트에게 사진 찍기 이펙트를 동기화
	UFUNCTION(NetMulticast, Reliable)
	void MulticastTakePhoto(const FVector& Location);

	// 서버에게 환호하기를 요청
	UFUNCTION(Server, Reliable)
	void CheerUpOnServer(const FVector& Location);

	// 모든 클라이언트에게 환호하기 이펙트를 동기화
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCheerUp(const FVector& Location);

	/**
	 * 소유자(PlayerController)에게 발사체 발사를 요청합니다.
	 * @param Location 발사 시작 위치입니다.
	 * @param Direction 발사 방향입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MVE|Action")
	void RequestThrowProjectile(const FVector& Location, const FVector& Direction);
};
