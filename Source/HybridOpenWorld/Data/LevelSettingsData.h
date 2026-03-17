#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "LevelSettingsData.generated.h"

/**
 * 레벨 성격 정의
 */
class UCameraPresetDataAsset;

UENUM(BlueprintType)
enum class ELevelType : uint8 
{ 
	WorldMap    UMETA(DisplayName = "월드맵"), 
	Detailed    UMETA(DisplayName = "세부지역"), 
	Event       UMETA(DisplayName = "이벤트") 
};

USTRUCT(BlueprintType)
struct FLevelSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level", meta=(DisplayName="레벨 타입", ToolTip = "규칙 및 조작 방식"))
	ELevelType LevelType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level", meta=(DisplayName="참조 레벨", ToolTip = "레벨 파일 연결"))
	TSoftObjectPtr<UWorld> LevelReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level", meta=(DisplayName="레벨 이름"))
	FString LevelName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn", meta=(DisplayName="기본 스폰 위치", ToolTip = "태그 정보 없을 때 나타날 기본 좌표"))
	FVector DefaultSpawnLocation= FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta=(DisplayName="태그별 스폰 목록", ToolTip = "포탈 태그와 일치하는 특정 좌표 데이터"))
	TMap<FGameplayTag, FVector> SpawnLocations;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta=(DisplayName="카메라 프리셋", ToolTip = "레벨에 적용될 카메라 프리셋 연결"))
	UCameraPresetDataAsset* CameraPreset = nullptr;
};