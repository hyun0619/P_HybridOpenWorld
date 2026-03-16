#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectHCameraActor.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;

/**
 * 각 레벨별 카메라 셋업 - 하이브리드 카메라
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Optimization")
	UWorldPartitionStreamingSourceComponent* StreamingSourceComponent;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm; // 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* MainCamera; // 메인 카메라

public:
	FVector GetCameraTargetLocation() const; // 카메라가 실제로 바라보고 있는 지점을 계산하여 반환
	void SetCameraMode(bool bIsWorldMap); // 월드맵 - 세부 레벨 카메라 설정 변경 함수
	
protected:
	UPROPERTY(EditAnywhere, Category="Camera|Settings")
	float WorldMapFOV = 30.0f; // 월드맵 FOV
	UPROPERTY(EditAnywhere, Category="Camera|Settings")
	float DetailedFOV = 90.0f; // 세부레벨 FOV
	UPROPERTY(EditAnywhere, Category="Camera|Settings")
	float WorldMapArmLength = 2000.0f; // 월드맵 스프링암 길이
	UPROPERTY(EditAnywhere, Category="Camera|Settings")
	float DetailedArmLength = 800.0f; // 세부레벨 스프링암 길이
};
