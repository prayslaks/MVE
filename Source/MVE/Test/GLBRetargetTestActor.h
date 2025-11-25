
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GLBRetargetTestActor.generated.h"

UCLASS()
class MVE_API AGLBRetargetTestActor : public AActor
{
	GENERATED_BODY()

public:
	AGLBRetargetTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ==================== 컴포넌트 ====================
    
    /** 소스 메시 (Mannequin - 미리 설정) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SourceMeshComponent;
    
    /** 타겟 메시 (런타임에 GLB 로드) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* TargetMeshComponent;

    // ==================== 설정 가능한 에셋 ====================
    
    /** 소스 메시 (Mannequin) - 에디터에서 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Setup")
    USkeletalMesh* SourceSkeletalMesh;
    
    /** 소스 애니메이션 - 테스트용 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Setup")
    UAnimSequence* SourceAnimation;
    
    /** 소스 IK Rig (Mannequin용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
    class UIKRigDefinition* SourceIKRig;

	/** 미리 만든 타겟 IK Rig (범용 Humanoid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
	class UIKRigDefinition* TargetIKRig;

	/** 미리 만든 IK Retargeter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
	class UIKRetargeter* RetargeterAsset;
    
    /** 타겟 AnimBP 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Setup")
    TSubclassOf<UAnimInstance> TargetAnimBPClass;
    
    /** GLB 파일 경로 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Setup")
    FString GLBFilePath = TEXT("Assets/Test/3d_character_15_.glb");

    // ==================== 런타임 생성 객체 ====================
    
	
    
    /** IK Retarget Processor */
    UPROPERTY()
    class UIKRetargetProcessor* RetargetProcessor;

    // ==================== 함수 ====================
    
    /** GLB 파일 로드 */
    UFUNCTION(BlueprintCallable, Category = "Runtime Loading")
    USkeletalMesh* LoadGLBFile(const FString& FilePath);
    
    /** 동적 리타게팅 설정 */
    UFUNCTION(BlueprintCallable, Category = "Runtime Loading")
    bool SetupDynamicRetargeting();
    
    /** 디버깅: 본 구조 출력 */
    void PrintBoneStructure(USkeletalMesh* Mesh, const FString& MeshName);
    
private:
    /** 로드된 GLB 메시 (GC 방지용 참조) */
    UPROPERTY()
    USkeletalMesh* LoadedTargetMesh;
};
