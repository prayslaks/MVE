// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnimNotify_CheckCombo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Template/Variant_Combat/Interfaces/CombatAttacker.h"

void UAnimNotify_CheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// cast the owner to the attacker interface
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp->GetOwner()))
	{
		// tell the actor to check for combo string
		AttackerInterface->CheckCombo();
	}
}

FString UAnimNotify_CheckCombo::GetNotifyName_Implementation() const
{
	return FString("Check Combo String");
}
