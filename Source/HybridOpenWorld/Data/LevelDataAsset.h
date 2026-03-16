// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

class UCameraPresetDataAsset;
/**
 * 레벨 정보
 */
UENUM(BlueprintType)
enum class ELevelType : uint8 { WorldMap, Detailed, Event };

UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API ULevelDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="Level")
	ELevelType LevelType; // 레벨 규칙 및 성격
	
	// 직접적인 UWorld* 참조는 데이터 에셋 로드 시 맵 전체를 메모리에 끌어올려 렉 유발
	UPROPERTY(EditAnywhere, Category="Level")
	TSoftObjectPtr<UWorld> LevelReference; // 레벨 데이터 파일을 가리키는 주소
	
	UPROPERTY(EditAnywhere, Category="Level")
	FString LevelName; // 레벨 이름
	
	UPROPERTY(EditAnywhere, Category="Spawn")
	FVector DefaultSpawnLocation; // 레벨 진입 시 스폰 지점
	
	UPROPERTY(EditAnywhere, Category = "Camera")
	UCameraPresetDataAsset* CameraPreset; // 카메라 프리셋 연결
};