#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CameraPresetDataAsset.h"
#include "CameraSettingsRow.generated.h"

/** 데이터 테이블의 한 줄(Row) — 레벨별 카메라 볼륨 설정을 일괄 관리 */
USTRUCT(BlueprintType)
struct FCameraSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 행이 대응하는 볼륨의 태그 (볼륨에서 같은 태그를 설정하면 자동 매칭) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(DisplayName="볼륨 태그"))
	FName VolumeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings Settings;

	/** 설명 메모 (에디터 전용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(DisplayName="기획 메모"))
	FString DesignerNote;
};