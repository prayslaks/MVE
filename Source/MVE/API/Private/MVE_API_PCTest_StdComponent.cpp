// Fill out your copyright notice in the Description page of Project Settings.

#include "API/Public/MVE_API_PCTest_StdComponent.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeAudioFunctionLibrary.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_Speaker.h"

UMVE_API_PCTest_StdComponent::UMVE_API_PCTest_StdComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	PRINTNETLOG(this, TEXT("UMVE_API_PCTest_StdComponent 생성 완료"));
}

void UMVE_API_PCTest_StdComponent::BeginPlay()
{
	Super::BeginPlay();
	PRINTNETLOG(this, TEXT("UMVE_API_PCTest_StdComponent BeginPlay 완료"));

	// 스테이지 레벨에 존재하는 모든 스피커에 접근한다
	TArray<AActor*> SpeakerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMVE_StageLevel_Speaker::StaticClass(), SpeakerActors);
	for (AActor* SpeakerActor : SpeakerActors)
	{
		if (AMVE_StageLevel_Speaker* Speaker = Cast<AMVE_StageLevel_Speaker>(SpeakerActor))
		{
			FoundSpeakers.Add(Speaker);
		}
	}
	PRINTNETLOG(this, TEXT("[SpeakerManager] Found %d speakers in the level on Role: %s"), FoundSpeakers.Num(), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
}

void UMVE_API_PCTest_StdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// 오디오 싱크 RPC 구현

void UMVE_API_PCTest_StdComponent::Client_PrepareAudio_Implementation(const FString& PresignedUrl)
{
	// This now runs on a specific client. This machine downloads the audio for itself.
	PRINTNETLOG(this, TEXT("[SpeakerManager] Client_PrepareAudio received on Role: %s. Downloading from URL: %s. PresignedUrl 전파 확인."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *PresignedUrl);
	if (PresignedUrl.IsEmpty() == false)
	{
		FglTFRuntimeHttpResponse Response;
		Response.BindUFunction(this, FName("OnAudioLoadedFromUrl"));
		
		const TMap<FString, FString> Headers;
		FglTFRuntimeConfig Config;
		Config.bAsBlob = true;
		UglTFRuntimeFunctionLibrary::glTFLoadAssetFromUrl(PresignedUrl, Headers, Response, Config);
	}
}

void UMVE_API_PCTest_StdComponent::OnAudioLoadedFromUrl(UglTFRuntimeAsset* Asset)
{
	// 모든 시스템에서 개별 다운로드가 종료되었을 때 이 콜백이 호출된다
	if (Asset == nullptr)
	{
		PRINTNETLOG(this, TEXT("[SpeakerManager] ERROR: Failed to load asset from URL on Role: %s."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
		return;
	}

	PRINTNETLOG(this, TEXT("[SpeakerManager] 각 클라이언트의 다운로드 완료. OnAudioLoadedFromUrl 호출됨."));

	constexpr EglTFRuntimeAudioDecoder Decoder {};
	const FglTFRuntimeAudioConfig Config{};

	// 런타임 변환에 성공했다면
	if (USoundWave* LoadedSoundWave = UglTFRuntimeAudioFunctionLibrary::LoadSoundFromBlob(Asset, Decoder, Config))
	{
		PRINTNETLOG(this, TEXT("[SpeakerManager] Successfully loaded sound on Role: %s. Setting on %d speakers."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), FoundSpeakers.Num());
		// Set the loaded sound on all speakers found by this machine.
		int32 SpeakerCount = 0;
		for (const AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
		{
			if (Speaker && Speaker->GetAudioComponent())
			{
				Speaker->GetAudioComponent()->SetSound(LoadedSoundWave);
				SpeakerCount++;
			}
		}
		PRINTNETLOG(this, TEXT("[SpeakerManager] Sound set on %d speakers. 각 클라이언트의 Speaker 준비 완료."), SpeakerCount);
	}
	else
	{
		PRINTNETLOG(this, TEXT("[SpeakerManager] ERROR: Failed to convert blob to SoundWave on Role: %s."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
	}
}

void UMVE_API_PCTest_StdComponent::Client_PlayPreparedAudio_Implementation()
{
	PRINTNETLOG(this, TEXT("[SpeakerManager] Client_PlayPreparedAudio received on Role: %s. Playing on %d speakers. Play 명령 전파 확인."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), FoundSpeakers.Num());
	int32 SpeakerCount = 0;
	for (AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
	{
		if (Speaker && Speaker->GetAudioComponent())
		{
			// We assume the sound has been prepared by MulticastPrepareAudio before this is called.
			Speaker->GetAudioComponent()->Play();
			SpeakerCount++;
		}
	}
	PRINTNETLOG(this, TEXT("[SpeakerManager] Play command sent to %d speakers. 각 클라이언트 Speaker의 재생 개시."), SpeakerCount);
}

void UMVE_API_PCTest_StdComponent::Client_StopPreparedAudio_Implementation()
{
	PRINTNETLOG(this, TEXT("[SpeakerManager] Client_StopPreparedAudio received on Role: %s. Stopping %d speakers."), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), FoundSpeakers.Num());
	int32 SpeakerCount = 0;
	for (AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
	{
		if (Speaker && Speaker->GetAudioComponent())
		{
			Speaker->GetAudioComponent()->Stop();
			SpeakerCount++;
		}
	}
	PRINTNETLOG(this, TEXT("[SpeakerManager] Stop command sent to %d speakers."), SpeakerCount);
}