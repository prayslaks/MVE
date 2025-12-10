
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVE_AUD_InteractionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class MVE_API UMVE_AUD_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UMVE_AUD_InteractionComponent();

    UPROPERTY(EditDefaultsOnly, Category = "Animation|LightStick")
    UAnimMontage* WaveLightStickMontage;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================
    // Public Interface
    // ============================================
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RequestWaveLightStick();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RequestEmote(int32 EmoteID);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RequestReaction(int32 ReactionID);

protected:
    virtual void BeginPlay() override;

    // ============================================
    // Light Stick Wave
    // ============================================
    
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerWaveLightStick();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastWaveLightStick();

    void PlayWaveLightStickAnimation();

    // ============================================
    // Emote System
    // ============================================
    
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPlayEmote(int32 EmoteID);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayEmote(int32 EmoteID);

    void PlayEmoteAnimation(int32 EmoteID);

    // ============================================
    // Reaction System (향후 확장)
    // ============================================
    
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPlayReaction(int32 ReactionID);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayReaction(int32 ReactionID);

private:
    // ============================================
    // References
    // ============================================
    
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // ============================================
    // Animation Assets
    // ============================================

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Emotes")
    TMap<int32, UAnimMontage*> EmoteMontages;

    // ============================================
    // VFX & SFX
    // ============================================
    
    UPROPERTY(EditDefaultsOnly, Category = "VFX|LightStick")
    UParticleSystem* WaveLightStickParticle;

    UPROPERTY(EditDefaultsOnly, Category = "SFX|LightStick")
    USoundBase* WaveLightStickSound;

    // ============================================
    // Cooldown System
    // ============================================
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|Cooldown")
    float WaveCooldown = 2.f;

    UPROPERTY(EditDefaultsOnly, Category = "Config|Cooldown")
    float EmoteCooldown = 3.f;

    float LastWaveTime = 0.f;
    float LastEmoteTime = 0.f;

    // ============================================
    // State
    // ============================================
    
    UPROPERTY(Replicated)
    bool bIsPerformingAction = false;

    // ============================================
    // Helper Functions
    // ============================================
    
    bool CanPerformAction() const;
    void InitializeReferences();};
