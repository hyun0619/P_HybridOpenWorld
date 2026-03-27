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

	// ─── Public API ───────────────────────────

	/** 현재 적용될 카메라 설정 반환 (실시간 LocalSettings) */
	FCameraPresetSettings GetCameraSettings() const { return LocalSettings; }

	/** 볼륨의 월드 공간 바운드 박스 반환 (패딩 미적용, 로컬 오프셋 적용) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
	FBox GetVolumeBounds() const;

	/** 볼륨의 논리적 중심점 (ActorLocation + VolumeOffset) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
	FVector GetVolumeCenter() const;

	/** 볼륨 BoxComponent의 스케일된 Extent */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
	FVector GetVolumeExtent() const;

	/** 런타임에 설정 변경 */
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
	// 00. Camera Settings (영구 저장 및 동기화 툴)
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, Category="00. Camera Settings", meta=(DisplayName="연동할 DA 원본 (영구 저장용)"))
	UCameraPresetDataAsset* LinkedDataAsset;

	UFUNCTION(CallInEditor, Category="00. Camera Settings", meta=(DisplayName="1. DA 원본 불러오기"))
	void LoadFromDataAsset();

	UFUNCTION(CallInEditor, Category="00. Camera Settings", meta=(DisplayName="2. DA 원본에 덮어쓰기 (저장)"))
	void SaveToDataAsset();

	UPROPERTY(EditAnywhere, Category="00. Camera Settings", meta=(DisplayName="우선순위 (Priority)", ToolTip="높을수록 우선 적용"))
	int32 Priority = 0;

	/** 뷰포트에서 직접 깎으면서 테스트할 로컬 세팅값 */
	UPROPERTY(EditAnywhere, Category="00. Camera Settings", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings LocalSettings;

	// ══════════════════════════════════════════
	// 01. 볼륨 레이아웃 (에디터에서 볼륨 형태 조정)
	// ══════════════════════════════════════════

	/** 볼륨 중심 오프셋 (액터 원점 대비 CollisionBox를 이동) */
	UPROPERTY(EditAnywhere, Category="01. Volume Layout", meta=(DisplayName="볼륨 중심 오프셋", ToolTip="CollisionBox의 중심을 액터 원점에서 이동합니다"))
	FVector VolumeOffset = FVector::ZeroVector;

	/** CollisionBox 크기 (반 익스텐트) */
	UPROPERTY(EditAnywhere, Category="01. Volume Layout", meta=(DisplayName="볼륨 크기 (Half Extent)"))
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

#if WITH_EDITOR
	/** 에디터에서 바운드 영역을 시각화 */
	void DrawBoundsPreview() const;
#endif
};