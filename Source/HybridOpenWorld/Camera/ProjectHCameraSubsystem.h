#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectHCameraSubsystem.generated.h"


class UCameraPresetDataAsset;

USTRUCT() // 스택에 저장할 데이터 구조체
struct FCameraStackEntry
{
	GENERATED_BODY()

	UPROPERTY()
	UCameraPresetDataAsset* Preset = nullptr;
	UPROPERTY()
	int32 Priority = 0;

	// 우선순위가 높은 것이 뒤로 가도록 정렬 기준 정의
	bool operator<(const FCameraStackEntry& Other) const { return Priority < Other.Priority; }
};

UCLASS()
class HYBRIDOPENWORLD_API UProjectHCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 볼륨에 진입했을 때 호출 (프리셋을 스택에 추가)
	void PushCameraPreset(UCameraPresetDataAsset* Preset, int32 Priority);
	// 볼륨에서 나갔을 때 호출 (프리셋을 스택에서 제거)
	void PopCameraPreset(UCameraPresetDataAsset* Preset);
	// [추가] 레벨의 기본 프리셋을 설정하는 함수
	void SetDefaultPreset(UCameraPresetDataAsset* InDefault);
	// [수정] 현재 활성화된 프리셋 가져오기
	UCameraPresetDataAsset* GetActivePreset() const;

private:
	// [추가] 스택이 비었을 때 돌아갈 기본 프리셋 저장용
	UPROPERTY()
	UCameraPresetDataAsset* DefaultLevelPreset = nullptr;
	// 카메라 설정들이 쌓이는 바구니(스택)
	UPROPERTY()
	TArray<FCameraStackEntry> CameraStack;
};