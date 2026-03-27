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

	/**
	 * ★ 핵심 수정: 실제 카메라가 바라보는 방향의 회전값을 반환
	 * 캐릭터의 WASD 이동 방향 계산에 사용해야 합니다.
	 * GetActorRotation()은 카메라 액터의 루트 위치 회전이고,
	 * 이 함수는 SpringArm의 월드 회전(=실제 화면에 보이는 방향)을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	FRotator GetCameraViewRotation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	UCameraComponent* GetMainCamera() const { return MainCamera; }

	UFUNCTION(BlueprintCallable, Category="Camera|Debug")
	void SetDebugDrawEnabled(bool bEnabled) { bDrawDebug = bEnabled; }

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
	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	float CameraRotationLagSpeed = 10.0f;

private:
	UPROPERTY()
	AActor* LastVolume = nullptr;

	bool bIsFirstTick = true;
	bool bDrawDebug = false;
	float CurrentOrthoWidth = 2048.0f;

	FVector ClampToBounds(const FVector& TargetLocation,
		const AProjectHCameraVolume* Volume,
		const FCameraPresetSettings& Preset) const;

	void ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut);
	void ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DeltaTime, bool bHardCut);
	void ApplyCollisionSettings(const FCameraPresetSettings& Preset);
};