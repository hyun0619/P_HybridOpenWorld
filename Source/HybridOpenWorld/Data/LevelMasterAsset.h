// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelMasterAsset.generated.h"

class ULevelDataAsset;
/**
 * 프로젝트의 모든 레벨 데이터를 한곳에서 관리하는 마스터 에셋
 */
UCLASS()
class HYBRIDOPENWORLD_API ULevelMasterAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 모든 레벨 데이터 에셋
	UPROPERTY(EditAnywhere, Category = "Levels")
	TArray<ULevelDataAsset*> AllLevelDatas;
};
