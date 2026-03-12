// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

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
	ELevelType LevelType;
	
	// 직접적인 UWorld* 참조는 데이터 에셋 로드 시 맵 전체를 메모리에 끌어올려 렉 유발
	UPROPERTY(EditAnywhere, Category="Level")
	TSoftObjectPtr<UWorld> LevelReference;
	
	UPROPERTY(EditAnywhere, Category="Level")
	FString LevelName;
	
	UPROPERTY(EditAnywhere, Category="Spawn")
	FVector DefaultSpawnLocation;
};
