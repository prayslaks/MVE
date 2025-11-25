// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnimNotify_DoAttackTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Template/Variant_Combat/Interfaces/CombatAttacker.h"

void UAnimNotify_DoAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// cast the owner to the attacker interface
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp->GetOwner()))
	{
		AttackerInterface->DoAttackTrace(AttackBoneName);
	}
}

FString UAnimNotify_DoAttackTrace::GetNotifyName_Implementation() const
{
	return FString("Do Attack Trace");
}