// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

/**
 * 레벨별 카메라 프리셋 데이터
 */
UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="Camera|Settings", meta=(DisplayName="카메라 암 길이" ,ToolTip = "카메라와 캐릭터 사이의 거리"))
	float TargetArmLength = 1000.f;
	UPROPERTY(EditAnywhere, Category = "Camera|Settings", meta=(DisplayName="시야각 (FOV)", ToolTip = "값 클수록 더 넓은 영역 봄"))
	float FieldOfView = 90.0f;
	UPROPERTY(EditAnywhere, Category = "Camera|Settings", meta=(DisplayName="고정 회전값", ToolTip = "카메라가 타겟을 바라보는 기본 각도"))
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Camera|Settings", meta=(DisplayName="카메라가 캐릭터를 따라가는 속도", ToolTip = "높을수록 즉각적으로 반응"))
	float TrackingInterpSpeed = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Camera|Movement", meta=(DisplayName="캐릭터 추적 여부", ToolTip = "체크하면 캐릭터를 따라가고, 해제하면 고정된 위치에서 촬영"))
	
	bool bFollowPawn = true;
	UPROPERTY(EditAnywhere, Category="Camera|Transition", meta=(DisplayName="카메라 전환 시간", ToolTip = "0이면 즉시 전환, 2.0이면 2초 동안 이동"))
	float BlendTime = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(DisplayName="틸트 쉬프트 활성화"))
	bool bEnableTiltShift = false;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="조리개 (F-Stop)", ToolTip = "값 낮을수록 초점 구역 외의 배경 더 강하게 흐려짐"))
	float ApertureFStop = 0.8f;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="센서 폭 (mm)", ToolTip = "값 클수록 심도가 얕아져 미니어처 효과 강조"))
	float SensorWidth = 144.0f;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="수동 초점 거리", ToolTip = "카메라로부터 초점이 가장 선명하게 맺히는 지점까지의 거리"))
	float ManualFocusDistance = 2000.f;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 가까운 블러 세기", ToolTip = "초점보다 앞에 있는 사물의 흐림 정도"))
	float NearBlurRadius = 15.f;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 먼 블러 세기", ToolTip = "초점보다 뒤에 있는 배경의 흐림 정도"))
	float FarBlurRadius = 15.f;
	UPROPERTY(EditAnywhere, Category = "Camera|TiltShift", meta=(EditCondition="bEnableTiltShift", DisplayName="블러 경계 부드러움 세기", ToolTip = "클수록 부드럽고 완만하게 흐려짐"))
	float FarTransitionRegion = 1000.f;
};
