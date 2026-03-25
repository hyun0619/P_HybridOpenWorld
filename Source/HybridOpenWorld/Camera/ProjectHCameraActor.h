#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectHCameraActor.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;

/**
 * 하이브리드 오픈월드용 카메라 액터
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraActor();
	
	void UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation); // 컨트롤러에서 넘겨준 수치로 카메라를 즉시 변경
	void UpdatePostProcessSettings(float InFocalDist, float InFStop, float InSensorWidth, float InNearBlur, float InFarBlur, float InFarTransition); // 컨트롤러에서 넘겨준 수치로 포스트 프로세스 업데이트
	FVector GetCameraTargetLocation() const; // 스트리밍 소스가 바라볼 지점 계산
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm; // 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* MainCamera; // 메인 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Optimization")
	UWorldPartitionStreamingSourceComponent* StreamingSourceComponent;

protected:
	UPROPERTY(EditAnywhere, Category = "Camera|Lag")
	bool bUseCameraLag = true; // 카메라 부드러움 설정
	UPROPERTY(EditAnywhere, Category = "Camera|Lag")
	float CameraLagSpeed = 3.0f; // 낮을수록 더 부드럽고 느리게 따라옴 (보통 3~10 사이)
	UPROPERTY(EditAnywhere, Category = "Camera|Lag")
	float CameraRotationLagSpeed = 10.0f; // 회전 시 부드러움
	
private:
	// 이전에 적용되었던 프리셋을 기억하는 변수
	UPROPERTY()
	class UCameraPresetDataAsset* LastPreset = nullptr;

	// 게임 시작 후 첫 프레임인지 확인하는 변수
	bool bIsFirstTick = true;
};
