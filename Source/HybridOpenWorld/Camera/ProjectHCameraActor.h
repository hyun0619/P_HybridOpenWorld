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
	
	FVector GetCameraTargetLocation() const; // 스트리밍 소스가 바라볼 지점 계산
	
protected:
	virtual void BeginPlay() override;
	
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
};
