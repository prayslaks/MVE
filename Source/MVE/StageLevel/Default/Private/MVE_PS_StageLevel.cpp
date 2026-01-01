// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_PS_StageLevel.h"

#include "MVE.h"
#include "Net/UnrealNetwork.h"

AMVE_PS_StageLevel::AMVE_PS_StageLevel()
{
	SetReplicates(true);
}

void AMVE_PS_StageLevel::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMVE_PS_StageLevel, bIsAudioReady);
}

void AMVE_PS_StageLevel::Server_SetIsAudioReady_Implementation(const bool Value)
{
	if (HasAuthority())
	{
		bIsAudioReady = Value;	
	}
}

void AMVE_PS_StageLevel::OnRep_IsAudioReady() const
{
	PRINTNETLOG(this, TEXT("음원이 준비됐습니다!"));
}
