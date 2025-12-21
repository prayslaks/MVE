#include "StageLevel/Actor/Public/MVE_StageLevel_AudCamera.h"

#include "MVE.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "StageLevel/Default/Public/MVE_GM_StageLevel.h"

AMVE_StageLevel_AudCamera::AMVE_StageLevel_AudCamera()
{
	//비활성화
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	
	// 스태틱 메시 컴포넌트 추가
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	
	// 플래시 방향을 나타내는 애로우 컴포넌트 추가
	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(StaticMeshComp);
	ArrowComp->SetRelativeLocation({-7.49f, 0.0f, 9.22f});
	ArrowComp->SetRelativeRotation({0.0f, 90.0f, 0.0f});
	ArrowComp->ArrowSize = 0.4f;
	
	// 스포트 라이트 컴포넌트 추가
	SpotLightComp = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComp"));
	SpotLightComp->SetupAttachment(StaticMeshComp);
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

void AMVE_StageLevel_AudCamera::TakePhoto(const AController* FlashMan)
{
	PRINTNETLOG(this, TEXT("사진 촬영 효과 실행!"));
	
	// 모든 클라이언트에서 시각/청각 효과 재생
	if (FlashTimelineComp)
	{
		FlashTimelineComp->PlayFromStart();
	}
	
	if (AudioComp)
	{
		AudioComp->Play();
	}

	// 게임플레이 로직은 서버에서만 실행
	if (HasAuthority())
	{
		if (AMVE_GM_StageLevel* GM = GetWorld()->GetAuthGameMode<AMVE_GM_StageLevel>())
		{
			TArray<AActor*> IgnoreActors;
			IgnoreActors.Emplace(this);
			
			// 플래시 효과 연산에는 애로우 컴포넌트 활용
			const FVector FlashLocation = ArrowComp->GetComponentLocation();
			const FVector FlashDirection = ArrowComp->GetForwardVector();
			
			// 게임모드에 플래시 효과 처리를 요청하고, 누가 플래시를 터뜨렸는지 알림
			GM->HandleFlashEffect(FlashMan, IgnoreActors, FlashLocation, FlashDirection, FlashEffectiveDistance, FlashAngleDotThreshold);
		}
	}
}

void AMVE_StageLevel_AudCamera::OnFlashUpdate(const float Value) const
{
	// 커브 값에 따라 스포트라이트 밝기 조절
	SpotLightComp->SetIntensity(Value * FlashIntensityMultiplier);
}