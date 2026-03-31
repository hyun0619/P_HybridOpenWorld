#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraActor.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;
class AProjectHCameraVolume;

/*
 * 게임 내 실제 메인 카메라 역할, 서브시스템의 신호 -> 셋팅 보간 처리
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectHCameraActor();
	
	void UpdateCameraSettings(float TargetArmLength, float FOV, FRotator Rotation); // 외부에서 직접 카메라 수치 제어
	void UpdatePostProcessSettings(bool bEnable, float InFocalDist, float InFStop, float InSensorWidth,
		float InNearBlur, float InFarBlur, float InFarTransition); // 포스트 프로세스 효과 실시간 업데이트
	
	/* 카메라가 현재 바라보는 지점의 월드 좌표 계산 */
	FVector GetCameraTargetLocation() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	FRotator GetCameraViewRotation() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera")
	UCameraComponent* GetMainCamera() const;
	
	void SetEdgeScrollOffset(const FVector& Offset); // 마우스 룩어라운드 기능을 위한 오프셋 설정

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/* 컴포넌트 구성 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm; // 카메라 거리 및 레그 제어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* MainCamera; // 실제 렌더링 담당 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Optimization")
	UWorldPartitionStreamingSourceComponent* StreamingSourceComponent; // 훨드 파티션 환경에서 카메라 주변 로딩 설정

	/* 기본 카메라 레그 설정 */
	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	bool bUseCameraLag = true;
	UPROPERTY(EditAnywhere, Category="Camera|Lag")
	float CameraLagSpeed = 3.0f;

private:
	UPROPERTY()
	AActor* LastVolume = nullptr; // 이젠 프레임의 볼륨 기억 -> 변경 여부 판단 
	
	bool bIsFirstTick = true; // 시작하자마자 카메라가 튀는 것을 방지하기 위한 플래그
	float CurrentOrthoWidth = 2048.0f;
	
	FVector EdgeScrollOffset = FVector::ZeroVector; // 룩어라운드에 의한 카메라 오픈셋
	
	/* SRP 개선을 위한 헬퍼 함수 */
	FVector ComputeTargetLocation(const FCameraPresetSettings& Preset,
		AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume, APawn* PlayerPawn) const; // 목표 위치 계산
	FRotator ComputeTargetRotation(const FCameraPresetSettings& Preset,
		AActor* ActiveInstigator, AProjectHCameraVolume* ActiveVolume) const; // 목표 회전값 계산
	void ApplyHardCut(const FCameraPresetSettings& Preset,
		const FVector& TargetLoc, const FRotator& TargetRot, float TargetFOV, APlayerController* PC); // 순간 이동 적용
	void ApplySmooth(const FCameraPresetSettings& Preset, const FVector& TargetLoc,
		const FRotator& TargetRot, float TargetFOV, float CamSpeed, float DT); // 보간 적용
	void ApplyLagSettings(const FCameraPresetSettings& Preset, bool bHardCut); // 거리 래그 설정 적용
	void ApplyProjectionSettings(const FCameraPresetSettings& Preset, float DeltaTime, bool bHardCut); // 투영 모드 전환 및 직교 보간
};