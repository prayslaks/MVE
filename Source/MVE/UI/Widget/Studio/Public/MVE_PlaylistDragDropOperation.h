// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "MVE_API_ResponseData.h"
#include "MVE_PlaylistDragDropOperation.generated.h"

UCLASS()
class MVE_API UMVE_PlaylistDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그 중인 음악 데이터
	UPROPERTY()
	FAudioFile DraggedAudioData;

	// 드래그 시작 위치 인덱스
	UPROPERTY()
	int32 OriginalIndex = -1;

	// 드래그 중인 위젯 (시각적 표시용)
	UPROPERTY()
	TObjectPtr<UWidget> DraggedWidget;
};
