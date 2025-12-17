#include "StageLevel/Actor/Public/MVE_StageLevel_AudCamera.h"

#include "MVE.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

AMVE_StageLevel_AudCamera::AMVE_StageLevel_AudCamera()
{
	//비활성화
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	
	// 스태틱 메시 컴포넌트 추가
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	SetRootComponent(StaticMeshComp);
	
	// 스포트 라이트 컴포넌트 추가
	SpotLightComp = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComp"));
	SpotLightComp->SetupAttachment(RootComponent);
	SpotLightComp->SetIntensity(0.0f);
	SpotLightComp->SetVisibility(true);
	
	// 오디오 컴포넌트 추가
	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);
	
	// 타임라인 컴포넌트 추가
	FlashTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("FlashTimelineComp"));
}

void AMVE_StageLevel_AudCamera::BeginPlay()
{
	Super::BeginPlay();
	
	// 커브가 유효한지 확인하고 타임라인에 바인딩
	if (FlashCurve)
	{
		FOnTimelineFloat FlashUpdateCallback;
		FlashUpdateCallback.BindUFunction(this, FName("OnFlashUpdate"));
		FlashTimelineComp->AddInterpFloat(FlashCurve, FlashUpdateCallback);
	}
	
	// 사운드가 유효한지 확인하고 오디오 컴포넌트에 설정
	if (TakePhotoSound)
	{
		AudioComp->SetSound(TakePhotoSound);
	}
}

void AMVE_StageLevel_AudCamera::TakePhoto() const
{
	PRINTNETLOG(this, TEXT("사진 촬영 효과 실행!"));
	
	// 타임라인 재생 시작
	if (FlashTimelineComp)
	{
		FlashTimelineComp->PlayFromStart();
	}
	
	// 사진 찍기 사운드 재생
	if (AudioComp)
	{
		AudioComp->Play();
	}	
}

void AMVE_StageLevel_AudCamera::OnFlashUpdate(const float Value) const
{
	// 커브 값에 따라 스포트라이트 밝기 조절
	SpotLightComp->SetIntensity(Value * FlashIntensityMultiplier);
}