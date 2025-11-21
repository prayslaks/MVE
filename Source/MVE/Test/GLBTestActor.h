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
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	TSubclassOf<UAnimInstance> AnimBPClass;
	
	// Processor를 멤버 변수로 보관
	UPROPERTY()
	class UIKRetargetProcessor* RetargetProcessor;

	void CopyPoseFromSourceToTarget();

	// 리타게팅 활성화 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	bool bEnableRetargeting = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	UAnimSequence* SourceAnim;
	
	// Mixamo 소스 IK Rig (에디터에서 만든 것)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	class UIKRigDefinition* SourceIKRig;
	
	// 에디터에서 설정할 표준 IK Rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	class UIKRigDefinition* StandardIKRig;
	
	// GLB 파일 경로 (Content 폴더 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTF Test")
	FString GLBFilePath;

	// IK Retargeter 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Retargeting")
	class UIKRetargeter* RetargeterAsset;
    
	// 스켈레탈 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "glTF Test")
	USkeletalMeshComponent* SourceMeshComponent;
    
	// 리타게팅된 타겟 메시 컴포넌트 (Mannequin)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* TargetMeshComponent;
    
	// GLB 로드 함수
	UFUNCTION(BlueprintCallable, Category = "glTF Test")
	USkeletalMesh* LoadGLBFile();

	// 테스트 함수
	UFUNCTION(BlueprintCallable, Category = "IK Retargeting")
	void LoadAndTestRetargeting();
	
	// 실제 리타게팅 수행
	bool PerformRetargeting();
    
	// 콜백 함수
	void OnSkeletalMeshLoaded(USkeletalMesh* LoadedMesh);

	// 본 매핑 출력 (디버깅용)
	void PrintBoneStructure(USkeletalMesh* Mesh);
};

