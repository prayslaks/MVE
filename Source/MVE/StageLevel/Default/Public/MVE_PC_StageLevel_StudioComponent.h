// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_PC_StageLevel_StudioComponent.generated.h"

class AMVE_StageLevel_Speaker;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MVE_API UMVE_PC_StageLevel_StudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVE_PC_StageLevel_StudioComponent();
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// 레벨에 배치된 스피커 액터들의 목록
	UPROPERTY()
	TArray<TObjectPtr<AMVE_StageLevel_Speaker>> FoundSpeakers;
	
public:
	// 서버로부터 오디오 준비 명령을 수신
	UFUNCTION(Client, Reliable)
	void Client_PrepareAudio(const FString& PresignedUrl);

	// Presigned URL에서 오디오 로드가 완료되었을 때 호출되는 콜백 함수
	UFUNCTION()
	void OnAudioLoadedFromUrl(class UglTFRuntimeAsset* Asset);
	
	// 서버로부터 오디오 재생 명령을 수신
	UFUNCTION(Client, Reliable)
	void Client_PlayPreparedAudio();

	// 서버로부터 오디오 중지 명령을 수신
	UFUNCTION(Client, Reliable)
	void Client_StopPreparedAudio();
};