#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraVolume.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(HideCategories=(Rendering, Replication, Input, Actor, LOD, Cooking))
class HYBRIDOPENWORLD_API AProjectHCameraVolume : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraVolume();
	virtual void OnConstruction(const FTransform& Transform) override;

	FCameraPresetSettings GetCameraSettings() const { return LocalSettings; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
	FVector GetVolumeCenter() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
	FVector GetVolumeExtent() const;

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyPreviewPostProcessing();

	// ══════════════════════════════════════════
	// 카메라 설정 > 01. 에셋 관리
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, Category="카메라 설정|01. 에셋 관리", meta=(DisplayName="연동할 DA 원본"))
	UCameraPresetDataAsset* LinkedDataAsset;

	UFUNCTION(CallInEditor, Category="카메라 설정|01. 에셋 관리", meta=(DisplayName="DA 불러오기"))
	void LoadFromDataAsset();

	UFUNCTION(CallInEditor, Category="카메라 설정|01. 에셋 관리", meta=(DisplayName="DA에 저장하기"))
	void SaveToDataAsset();

	UPROPERTY(EditAnywhere, Category="카메라 설정|01. 에셋 관리",
		meta=(DisplayName="우선순위", ToolTip="높을수록 우선 적용"))
	int32 Priority = 0;

	// ══════════════════════════════════════════
	// 카메라 설정 > 02. 세부 설정
	// (LocalSettings의 내부 카테고리 1/2/3이 하위에 표시됨)
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, Category="카메라 설정|02. 세부 설정", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings LocalSettings;

	// ══════════════════════════════════════════
	// 카메라 설정 > 03. 볼륨 레이아웃
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, Category="카메라 설정|03. 볼륨 레이아웃",
		meta=(DisplayName="볼륨 크기 (Half Extent)"))
	FVector VolumeExtent = FVector(500.f, 500.f, 200.f);

	// ══════════════════════════════════════════
	// 내부 컴포넌트
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category="Internal Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, Category="Internal Components")
	USpringArmComponent* PreviewSpringArm;

	UPROPERTY(VisibleAnywhere, Category="Internal Components")
	UCameraComponent* PreviewCamera;
};