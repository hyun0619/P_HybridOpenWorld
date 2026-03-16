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
	
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float FieldOfView = 90.0f; // FOV
	
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f); // 회전값
	
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float TrackingInterpSpeed = 5.0f; // 카메라가 캐릭터를 따라가는 속도
	
	UPROPERTY(EditAnywhere, Category = "Camera|Movement")
	bool bFollowPawn = true; // 체크해제 - 고정 카메라
};
