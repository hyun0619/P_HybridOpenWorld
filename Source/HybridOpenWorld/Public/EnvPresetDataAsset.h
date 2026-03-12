// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnvPresetDataAsset.generated.h"

/**
 * 환경 프리셋 데이터
 */
UCLASS()
class HYBRIDOPENWORLD_API UEnvPresetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 해당 프리셋이 적용될 시간 (예: 낮=12.0, 밤=0.0)
	UPROPERTY(EditAnywhere, Category="Time")
	float TargetTime;
	
	// 라이팅 데이터
	UPROPERTY(EditAnywhere, Category="Lighting")
	FLinearColor SunColor;
	UPROPERTY(EditAnywhere, Category="Lighting")
	float SunIntensity;
	UPROPERTY(EditAnywhere, Category="Lighting")
	FRotator SunRotation;
	
	// 안개 및 대기
	UPROPERTY(EditAnywhere, Category="Atmosphere")
	float FogDensity;
	// MPC(머티리얼 파라미터 컬렉션)와 연동할 젖음 값
	UPROPERTY(EditAnywhere, Category="Material")
	float GlobalWetness;
};
