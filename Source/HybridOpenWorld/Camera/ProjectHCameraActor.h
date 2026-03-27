#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraActor.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;
class AProjectHCameraVolume;

/**
 * 하이브리드 오픈월드용 카메라 액터
 * 
 * 서브시스템의 활성 프리셋에 따라 위치, 회전, FOV, 투영 모드를 매 프레임 갱신합니다.
 * 바운드 클램핑, 충돌 감지, 래그 설정을 볼륨별로 독립 적용합니다.
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraActor();

	/** 외부에서 카메라 설정 직접 변경 (레거시 호환) */
	void UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation);
	/** 포스트 프로세스 갱신 */
	void UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop, float InSensorWidth,
		float InNearBlur, float InFarBlur, float InFarTransition);
	/** 스트리밍 소스가 바라볼 지점 */
	FVector GetCameraTargetLocation() const;

	/** 카메라 컴포넌트 접근 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	UCameraComponent* GetMainCamera() const { return MainCamera; }

	/** 전환 진행 중인지 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	bool IsTransitioning() const { return bIsTransitioning; }

	/** 디버그 시각화 토글 */
	UFUNCTION(BlueprintCallable, Category="Camera|Debug")
	void SetDebugDrawEnabled(bool bEnabled) { bDrawDebug = bEnabled; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// ─── 컴포넌트 ─────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* MainCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Optimization")
	UWorldPartitionStreamingSourceComponent* StreamingSourceComponent;

	// ─── 래그 기본 설정 ───────────────────────

	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	bool bUseCameraLag = true;

	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	float CameraLagSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	float CameraRotationLagSpeed = 10.0f;

	// ─── Blueprint Override Points ────────────

	/** 블루프린트에서 최종 카메라 위치를 추가 조정할 기회 */
	UFUNCTION(BlueprintNativeEvent, Category="Camera")
	FVector BP_ModifyTargetLocation(const FVector& ComputedLocation, const FCameraPresetSettings& ActivePreset) const;
	virtual FVector BP_ModifyTargetLocation_Implementation(const FVector& ComputedLocation,
		const FCameraPresetSettings& ActivePreset) const { return ComputedLocation; }

private:
	// ─── 상태 ─────────────────────────────────

	UPROPERTY()
	AActor* LastVolume = nullptr;

	bool bIsFirstTick = true;
	bool bIsTransitioning = false;
	bool bDrawDebug = false;

	/** 현재 보간 중인 OrthoWidth */
	float CurrentOrthoWidth = 2048.0f;

	// ─── 핵심 로직 ────────────────────────────

	/**
	 * 바운드 클램핑: 카메라 타겟 위치를 볼륨 경계 내로 제한
	 * 볼륨의 로컬 공간에서 클램프하므로 회전된 볼륨도 올바르게 처리
	 */
	FVector ClampToBounds(const FVector& TargetLocation,
		const AProjectHCameraVolume* Volume,
		const FCameraPresetSettings& Preset) const;

	/** SpringArm 래그 설정을 프리셋에 맞게 갱신 */
	void ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut);

	/** 투영 모드 및 OrthoWidth 갱신 */
	void ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DeltaTime, bool bHardCut);

	/** 충돌 설정 갱신 */
	void ApplyCollisionSettings(const FCameraPresetSettings& Preset);

#if WITH_EDITOR
	/** 디버그 정보 표시 */
	void DrawDebugInfo(const FVector& TargetLoc, const AProjectHCameraVolume* Volume,
		const FCameraPresetSettings& Preset) const;
#endif
};