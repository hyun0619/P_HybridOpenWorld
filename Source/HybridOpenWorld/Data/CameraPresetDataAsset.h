// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

/**
 * 레벨별 카메라 프리셋 데이터
 */
UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="Camera|Settings")
	float TargetArmLength = 1000.f; // 타겟 거리
	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	float FieldOfView = 90.0f; // FOV
	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f); // 회전값
	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	float TrackingInterpSpeed = 5.0f; // 카메라가 캐릭터를 따라가는 속도
	UPROPERTY(EditAnywhere, Category = "Camera|Movement")
	bool bFollowPawn = true; // 체크해제 - 고정 카메라
	
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift")
	bool bEnableTiltShift = false; // 틸트 쉬프트
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float ApertureFStop = 0.8f; // 조리개 - 낮을수록 주변이 더 흐려짐
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float SensorWidth = 144.0f; // 센서 폭 - 클수록 심도가 얕아짐
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float ManualFocusDistance = 2000.f; // 초점이 맞는 거리
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float NearBlurRadius = 15.f; // 카메라와 가까운 블러 세기
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float FarBlurRadius = 15.f; // 카메라와 먼 블러 세기
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift"))
	float FarTransitionRegion = 1000.f; // 초점 구역에서 먼 블러 구역으로 전환되는 구간의 길이 - 클수록 부드럽고 완만하게 흐려짐
};
