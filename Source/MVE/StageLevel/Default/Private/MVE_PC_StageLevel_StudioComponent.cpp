#include "StageLevel/Default/Public/MVE_PC_StageLevel_StudioComponent.h"

#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeAudioFunctionLibrary.h"
#include "MVE.h"
#include "Kismet/GameplayStatics.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_Speaker.h"

UMVE_PC_StageLevel_StudioComponent::UMVE_PC_StageLevel_StudioComponent()
{
	// 이 컴포넌트가 매 프레임 Tick을 호출하도록 설정합니다.
	PrimaryComponentTick.bCanEverTick = true;
	// 네트워크 리플리케이션을 활성화합니다.
	SetIsReplicatedByDefault(true);
	PRINTNETLOG(this, TEXT("StdComponent 생성 완료"));
}

void UMVE_PC_StageLevel_StudioComponent::BeginPlay()
{
	Super::BeginPlay();
	PRINTNETLOG(this, TEXT("StdComponent BeginPlay 완료"));

	// 레벨에 있는 모든 스피커 액터를 찾아 배열에 저장합니다.
	// 이 코드는 서버와 모든 클라이언트에서 각각 실행됩니다.
	TArray<AActor*> SpeakerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMVE_StageLevel_Speaker::StaticClass(), SpeakerActors);
	for (AActor* SpeakerActor : SpeakerActors)
	{
		if (AMVE_StageLevel_Speaker* Speaker = Cast<AMVE_StageLevel_Speaker>(SpeakerActor))
		{
			FoundSpeakers.Add(Speaker);
		}
	}
	PRINTNETLOG(this, TEXT("[오디오 동기화] 레벨에서 %d개의 스피커를 찾았습니다. (실행 위치: %s)"), FoundSpeakers.Num(), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
}

void UMVE_PC_StageLevel_StudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// 서버가 호출 -> 특정 클라이언트에서 실행
void UMVE_PC_StageLevel_StudioComponent::Client_PrepareAudio_Implementation(const FString& PresignedUrl)
{
	// 이 함수는 서버의 요청을 받은 특정 클라이언트에서만 실행됩니다.
	// 클라이언트는 전달받은 URL로부터 오디오 파일을 직접 다운로드합니다.
	PRINTNETLOG(this, TEXT("[오디오 동기화] Presigned URL 수신 확인. 오디오 다운로드 시작. (URL: %s)"), *PresignedUrl);
	
	if (PresignedUrl.IsEmpty() == false)
	{
		FglTFRuntimeHttpResponse Response;
		Response.BindUFunction(this, FName("OnAudioLoadedFromUrl"));
		
		const TMap<FString, FString> Headers;
		FglTFRuntimeConfig Config;
		Config.bAsBlob = true; // 파일을 바이너리 데이터(Blob)로 받도록 설정
		UglTFRuntimeFunctionLibrary::glTFLoadAssetFromUrl(PresignedUrl, Headers, Response, Config);
	}
}

// 오디오 다운로드가 완료되면 glTFRuntime 플러그인이 이 함수를 호출합니다.
void UMVE_PC_StageLevel_StudioComponent::OnAudioLoadedFromUrl(UglTFRuntimeAsset* Asset)
{
	// 이 콜백은 각 클라이언트에서 개별 다운로드가 완료된 후 실행됩니다.
	if (!Asset)
	{
		PRINTNETLOG(this, TEXT("[오디오 동기화] 오류: URL에서 에셋을 로드하지 못했습니다."));
		return;
	}

	PRINTNETLOG(this, TEXT("[오디오 동기화] 각 클라이언트의 다운로드 완료. OnAudioLoadedFromUrl 호출됨."));

	// 다운로드된 바이너리 데이터를 사운드 웨이브 에셋으로 변환합니다.
	const EglTFRuntimeAudioDecoder Decoder{};
	const FglTFRuntimeAudioConfig Config{};
	USoundWave* LoadedSoundWave = UglTFRuntimeAudioFunctionLibrary::LoadSoundFromBlob(Asset, Decoder, Config);

	if (LoadedSoundWave)
	{
		PRINTNETLOG(this, TEXT("[오디오 동기화] 사운드 로드 성공. %d개의 스피커에 사운드를 설정합니다."), FoundSpeakers.Num());
		
		// 이 클라이언트가 찾은 모든 스피커에 로드된 사운드를 설정합니다.
		int32 SpeakerCount = 0;
		for (AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
		{
			if (Speaker && Speaker->GetAudioComponent())
			{
				Speaker->GetAudioComponent()->SetSound(LoadedSoundWave);
				SpeakerCount++;
			}
		}
		PRINTNETLOG(this, TEXT("[오디오 동기화] %d개의 스피커에 사운드 설정 완료. 재생 준비 완료."), SpeakerCount);
	}
	else
	{
		PRINTNETLOG(this, TEXT("[오디오 동기화] 오류: 바이너리 데이터를 SoundWave로 변환하지 못했습니다."));
	}
}

// 서버가 호출 -> 특정 클라이언트에서 실행
void UMVE_PC_StageLevel_StudioComponent::Client_PlayPreparedAudio_Implementation()
{
	PRINTNETLOG(this, TEXT("[오디오 동기화] 재생 명령 수신 확인. %d개의 스피커에서 오디오를 재생합니다."), FoundSpeakers.Num());
	int32 SpeakerCount = 0;
	for (AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
	{
		if (Speaker && Speaker->GetAudioComponent())
		{
			// 이전에 Client_PrepareAudio를 통해 사운드가 설정되었다고 가정하고 재생합니다.
			Speaker->GetAudioComponent()->Play();
			SpeakerCount++;
		}
	}
	PRINTNETLOG(this, TEXT("[오디오 동기화] %d개의 스피커에 재생 명령 전달 완료."), SpeakerCount);
}

// 서버가 호출 -> 특정 클라이언트에서 실행
void UMVE_PC_StageLevel_StudioComponent::Client_StopPreparedAudio_Implementation()
{
	PRINTNETLOG(this, TEXT("[오디오 동기화] 중지 명령 수신 확인. %d개의 스피커에서 오디오를 중지합니다."), FoundSpeakers.Num());
	int32 SpeakerCount = 0;
	for (AMVE_StageLevel_Speaker* Speaker : FoundSpeakers)
	{
		if (Speaker && Speaker->GetAudioComponent())
		{
			Speaker->GetAudioComponent()->Stop();
			SpeakerCount++;
		}
	}
	PRINTNETLOG(this, TEXT("[오디오 동기화] %d개의 스피커에 중지 명령 전달 완료."), SpeakerCount);
}