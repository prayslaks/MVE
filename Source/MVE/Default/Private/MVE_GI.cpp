// Fill out your copyright notice in the Description page of Project Settings.


#include "Default/Public/MVE_GI.h"

#include "MVE_API_Helper.h"

void UMVE_GI::Init()
{
	Super::Init();
	
	//서버 초기화 시도
	UMVE_API_Helper::Initialize();
}
