#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AvatarPreviewActor.generated.h"

class USceneCaptureComponent2D;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UTextureRenderTarget2D;
class UglTFRuntimeAsset;

UCLASS()
class MVE_API AAvatarPreviewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAvatarPreviewActor();

	virtual void Tick(float DeltaTime) override;

	// GLB 파일 로드
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void LoadAvatarMesh(const FString& FilePath);

	// StaticMesh 설정
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewMesh(UStaticMesh* Mesh);

	// SkeletalMesh 설정
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewSkeletalMesh(USkeletalMesh* SkeletalMesh);

	// RenderTarget 가져오기
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Preview")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	// 타겟 액터 (프리뷰할 메쉬를 가진 액터)
	UPROPERTY(BlueprintReadWrite, Category = "Preview")
	TObjectPtr<AActor> TargetActor;


	virtual void BeginPlay() override;

	// Scene Capture Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	// Render Target
	UPROPERTY(EditAnywhere, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;


	// 프리뷰 메쉬를 가진 액터
	UPROPERTY()
	TObjectPtr<AActor> PreviewMeshActor;

	// 로드된 GLTF 애셋
	UPROPERTY()
	TObjectPtr<UglTFRuntimeAsset> LoadedGLTFAsset;

	void CreateRenderTarget();
};