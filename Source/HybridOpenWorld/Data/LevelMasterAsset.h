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
	UPROPERTY(EditAnywhere, Category = "Levels", meta=(DisplayName="전체 레벨 목록", ToolTip = "모든 레벨 데이터 에셋"))
	TArray<ULevelDataAsset*> AllLevelDatas;
};
