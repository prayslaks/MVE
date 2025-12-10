#include "AutoGripComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UAutoGripComponent::UAutoGripComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAutoGripComponent::BeginPlay()
{
    Super::BeginPlay();

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMesh = OwnerCharacter->GetMesh();
    }

    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("손이 없어요"));
        return;
    }

    InitializeBoneNames();
    SetupDetectionSphere();
}

void UAutoGripComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopGripCheckTimer();

    if (DetectionSphere)
    {
        DetectionSphere->DestroyComponent();
    }

    Super::EndPlay(EndPlayReason);
}

void UAutoGripComponent::InitializeBoneNames()
{
    // 손 접미사
    FString Suffix = (HandSide == EGripHand::Left) ? TEXT("_l") : TEXT("_r");

    // 검지
    IndexBoneNames[0] = FName(*FString::Printf(TEXT("index_01%s"), *Suffix));
    IndexBoneNames[1] = FName(*FString::Printf(TEXT("index_02%s"), *Suffix));
    IndexBoneNames[2] = FName(*FString::Printf(TEXT("index_03%s"), *Suffix));

    // 중지
    MiddleBoneNames[0] = FName(*FString::Printf(TEXT("middle_01%s"), *Suffix));
    MiddleBoneNames[1] = FName(*FString::Printf(TEXT("middle_02%s"), *Suffix));
    MiddleBoneNames[2] = FName(*FString::Printf(TEXT("middle_03%s"), *Suffix));

    // 약지
    RingBoneNames[0] = FName(*FString::Printf(TEXT("ring_01%s"), *Suffix));
    RingBoneNames[1] = FName(*FString::Printf(TEXT("ring_02%s"), *Suffix));
    RingBoneNames[2] = FName(*FString::Printf(TEXT("ring_03%s"), *Suffix));

    // 새끼
    PinkyBoneNames[0] = FName(*FString::Printf(TEXT("pinky_01%s"), *Suffix));
    PinkyBoneNames[1] = FName(*FString::Printf(TEXT("pinky_02%s"), *Suffix));
    PinkyBoneNames[2] = FName(*FString::Printf(TEXT("pinky_03%s"), *Suffix));

    // 손목
    WristBoneName = FName(*FString::Printf(TEXT("hand%s"), *Suffix));

    // 소켓 이름도 설정
    GripSocketName = FName(*FString::Printf(TEXT("hand%s_socket"), *Suffix));

    UE_LOG(LogTemp, Log, TEXT(" %s 위치의 손가락"), 
        HandSide == EGripHand::Left ? TEXT("LEFT") : TEXT("RIGHT"));
}

void UAutoGripComponent::SetupDetectionSphere()
{
    DetectionSphere = NewObject<USphereComponent>(GetOwner(), TEXT("GripDetectionSphere"));
    if (!DetectionSphere)
    {
        return;
    }

    DetectionSphere->SetSphereRadius(DetectionRadius);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->SetGenerateOverlapEvents(true);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    if (OwnerMesh->DoesSocketExist(GripSocketName))
    {
        DetectionSphere->SetupAttachment(OwnerMesh, GripSocketName);
    }
    else
    {
        DetectionSphere->SetupAttachment(OwnerMesh, MiddleBoneNames[0]);
    }
    
    DetectionSphere->RegisterComponent();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UAutoGripComponent::OnDetectionBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UAutoGripComponent::OnDetectionEndOverlap);
}

void UAutoGripComponent::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!IsValidGrippableItem(OtherActor))
    {
        return;
    }

    if (GrippedItem)
    {
        return;
    }

    if (!OverlapCandidates.Contains(OtherActor))
    {
        OverlapCandidates.Add(OtherActor);
    }

    StartGripCheckTimer();
}

void UAutoGripComponent::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    OverlapCandidates.Remove(OtherActor);

    if (OverlapCandidates.Num() == 0 && !GrippedItem)
    {
        StopGripCheckTimer();
    }
}

bool UAutoGripComponent::IsValidGrippableItem(AActor* Actor) const
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }

    return Actor->ActorHasTag(GrippableTag);
}

void UAutoGripComponent::StartGripCheckTimer()
{
    if (bIsTimerActive)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            GripCheckTimerHandle,
            this,
            &UAutoGripComponent::OnGripCheckTimer,
            CheckInterval,
            true
        );
        bIsTimerActive = true;
    }
}

void UAutoGripComponent::StopGripCheckTimer()
{
    if (!bIsTimerActive)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GripCheckTimerHandle);
        bIsTimerActive = false;
    }
}

void UAutoGripComponent::OnGripCheckTimer()
{
    CurrentGripValue = CalculateGripValue();
    UpdateGripState();

    if (GrippedItem)
    {
        if (CurrentGripState == EGripState::Open)
        {
            DetachCurrentItem();
        }
    }
    else
    {
        if (CurrentGripState == EGripState::Closed && OverlapCandidates.Num() > 0)
        {
            AActor* ClosestItem = nullptr;
            float ClosestDist = FLT_MAX;
            FVector SocketLocation = OwnerMesh->GetSocketLocation(GripSocketName);

            for (AActor* Candidate : OverlapCandidates)
            {
                if (!Candidate) continue;

                float Dist = FVector::Dist(SocketLocation, Candidate->GetActorLocation());
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    ClosestItem = Candidate;
                }
            }

            if (ClosestItem)
            {
                TryAttachItem(ClosestItem);
            }
        }
    }

    if (OverlapCandidates.Num() == 0 && !GrippedItem)
    {
        StopGripCheckTimer();
    }
}

void UAutoGripComponent::UpdateControlTransform(int32 ControlIndex, const FTransform& InTransform, const FName& ControlName)
{
    // Control 이름에서 _ctrl 제거하여 본 이름 추출
    FString ControlStr = ControlName.ToString();
    FString BoneStr = ControlStr.Replace(TEXT("_ctrl"), TEXT(""));
    FName BoneName = FName(*BoneStr);

    ControlTransformMap.Add(BoneName, InTransform);
}

FTransform UAutoGripComponent::GetControlTransform(const FName& BoneName) const
{
    if (const FTransform* Found = ControlTransformMap.Find(BoneName))
    {
        return *Found;
    }
    return FTransform::Identity;
}

float UAutoGripComponent::CalculateGripValue() const
{
    if (ControlTransformMap.Num() == 0)
    {
        return 0.f;
    }

    float TotalCurl = 0.f;
    int32 ValidFingers = 0;

    float IndexCurl = CalculateFingerCurl(IndexBoneNames);
    float MiddleCurl = CalculateFingerCurl(MiddleBoneNames);
    float RingCurl = CalculateFingerCurl(RingBoneNames);
    float PinkyCurl = CalculateFingerCurl(PinkyBoneNames);

    if (IndexCurl >= 0.f) { TotalCurl += IndexCurl; ValidFingers++; }
    if (MiddleCurl >= 0.f) { TotalCurl += MiddleCurl; ValidFingers++; }
    if (RingCurl >= 0.f) { TotalCurl += RingCurl; ValidFingers++; }
    if (PinkyCurl >= 0.f) { TotalCurl += PinkyCurl; ValidFingers++; }

    if (ValidFingers == 0)
    {
        return 0.f;
    }

    return TotalCurl / ValidFingers;
}

float UAutoGripComponent::CalculateFingerCurl(const FName* FingerBoneNames) const
{
    // 손목 위치
    FTransform WristTransform = GetControlTransform(WristBoneName);
    FVector WristPos = WristTransform.GetLocation();

    // 손가락 관절 위치
    FVector Finger01Pos = GetControlTransform(FingerBoneNames[0]).GetLocation();
    FVector Finger02Pos = GetControlTransform(FingerBoneNames[1]).GetLocation();
    FVector Finger03Pos = GetControlTransform(FingerBoneNames[2]).GetLocation();

    // 유효성 검사
    if (Finger01Pos.IsZero() || Finger03Pos.IsZero())
    {
        return -1.f;
    }

    // 손 크기 정규화
    float HandScale = FVector::Dist(WristPos, Finger01Pos);
    if (HandScale < KINDA_SMALL_NUMBER)
    {
        return -1.f;
    }

    // 손가락 끝 - 첫번째 관절 거리
    float TipToBase = FVector::Dist(Finger03Pos, Finger01Pos);
    float NormalizedDist = TipToBase / HandScale;

    // 굽힘 값 (0=펴짐, 1=쥠)
    float CurlAmount = FMath::Clamp(1.f - (NormalizedDist - 0.2f) / 0.6f, 0.f, 1.f);

    return CurlAmount;
}

void UAutoGripComponent::UpdateGripState()
{
    EGripState PrevState = CurrentGripState;

    if (CurrentGripState == EGripState::Open)
    {
        if (CurrentGripValue >= GripThreshold)
        {
            CurrentGripState = EGripState::Gripping;
        }
    }
    else if (CurrentGripState == EGripState::Gripping)
    {
        if (CurrentGripValue >= GripThreshold + 0.2f)
        {
            CurrentGripState = EGripState::Closed;
        }
        else if (CurrentGripValue < GripThreshold - 0.1f)
        {
            CurrentGripState = EGripState::Open;
        }
    }
    else if (CurrentGripState == EGripState::Closed)
    {
        if (CurrentGripValue < ReleaseThreshold)
        {
            CurrentGripState = EGripState::Gripping;
        }
    }

    if (CurrentGripState == EGripState::Gripping && CurrentGripValue < GripThreshold - 0.2f)
    {
        CurrentGripState = EGripState::Open;
    }

    if (PrevState != CurrentGripState)
    {
        UE_LOG(LogTemp, Log, TEXT("상태: %d -> %d (잡고있는 수치: %.2f)"),
            (int32)PrevState, (int32)CurrentGripState, CurrentGripValue);
    }
}

void UAutoGripComponent::TryAttachItem(AActor* Item)
{
    if (!Item || !OwnerMesh)
    {
        return;
    }

    UPrimitiveComponent* ItemRoot = Cast<UPrimitiveComponent>(Item->GetRootComponent());
    if (ItemRoot)
    {
        ItemRoot->SetSimulatePhysics(false);
        ItemRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        true
    );

    Item->AttachToComponent(OwnerMesh, AttachRules, GripSocketName);

    GrippedItem = Item;
    OverlapCandidates.Remove(Item);

    OnItemGripped.Broadcast(Item);
}

void UAutoGripComponent::DetachCurrentItem()
{
    if (!GrippedItem)
    {
        return;
    }

    AActor* ReleasedItem = GrippedItem;

    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    GrippedItem->DetachFromActor(DetachRules);

    UPrimitiveComponent* ItemRoot = Cast<UPrimitiveComponent>(ReleasedItem->GetRootComponent());
    if (ItemRoot)
    {
        ItemRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ItemRoot->SetSimulatePhysics(true);
    }

    GrippedItem = nullptr;

    OnItemReleased.Broadcast(ReleasedItem);

    if (OverlapCandidates.Num() == 0)
    {
        StopGripCheckTimer();
    }
}

void UAutoGripComponent::ForceRelease()
{
    if (GrippedItem)
    {
        DetachCurrentItem();
    }
}

void UAutoGripComponent::ForceGrip(AActor* ItemToGrip)
{
    if (!ItemToGrip || !IsValidGrippableItem(ItemToGrip))
    {
        return;
    }

    if (GrippedItem)
    {
        DetachCurrentItem();
    }

    TryAttachItem(ItemToGrip);
}