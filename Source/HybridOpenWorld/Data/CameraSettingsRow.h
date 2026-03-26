#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CameraPresetDataAsset.h" // 기존 FCameraPresetSettings 포함
#include "CameraSettingsRow.generated.h"

// ★ 데이터 테이블의 한 줄(Row)이 될 구조체
USTRUCT(BlueprintType)
struct FCameraSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Camera")
	FCameraPresetSettings Settings;
};