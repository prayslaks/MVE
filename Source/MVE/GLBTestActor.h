// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GLBTestActor.generated.h"

UCLASS()
class MVE_API AGLBTestActor : public AActor
{
	GENERATED_BODY()

public:    
	AGLBTestActor();

protected:
	virtual void BeginPlay() override;

public:    
	// GLB 파일 경로 (Content 폴더 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTF Test")
	FString GLBFilePath;
    
	// 스켈레탈 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "glTF Test")
	USkeletalMeshComponent* SkeletalMeshComponent;
    
	// GLB 로드 함수
	UFUNCTION(BlueprintCallable, Category = "glTF Test")
	void LoadGLBFile();
    
	// 콜백 함수
	void OnSkeletalMeshLoaded(USkeletalMesh* LoadedMesh);
};
