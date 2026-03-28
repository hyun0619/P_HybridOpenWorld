#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraActor.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;
class AProjectHCameraVolume;

UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraActor();

	void UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation);
	void UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop, float InSensorWidth,
		float InNearBlur, float InFarBlur, float InFarTransition);
	FVector GetCameraTargetLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	FRotator GetCameraViewRotation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	UCameraComponent* GetMainCamera() const { return MainCamera; }

	/** 엣지스크롤 컴포넌트가 매 프레임 호출 — 카메라 위치에 오프셋 추가 */
	void SetEdgeScrollOffset(const FVector& Offset) { EdgeScrollOffset = Offset; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* MainCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Optimization")
	UWorldPartitionStreamingSourceComponent* StreamingSourceComponent;

	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	bool bUseCameraLag = true;
	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	float CameraLagSpeed = 3.0f;

private:
	UPROPERTY()
	AActor* LastVolume = nullptr;
	bool bIsFirstTick = true;
	float CurrentOrthoWidth = 2048.0f;

	/** 엣지스크롤에 의한 카메라 오프셋 (매 프레임 리셋) */
	FVector EdgeScrollOffset = FVector::ZeroVector;

	// ★ Tick에서 추출된 헬퍼 함수들 (SRP 개선)
	FVector ComputeTargetLocation(const FCameraPresetSettings& Preset,
		AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume, APawn* PlayerPawn) const;
	FRotator ComputeTargetRotation(const FCameraPresetSettings& Preset,
		AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume) const;
	void ApplyHardCut(const FCameraPresetSettings& Preset,
		const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, APlayerController* PC);
	void ApplySmooth(const FCameraPresetSettings& Preset,
		const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, float CamSpeed, float DT);

	void ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut);
	void ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DeltaTime, bool bHardCut);
};