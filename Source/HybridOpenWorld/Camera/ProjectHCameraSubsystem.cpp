#include "ProjectHCameraSubsystem.h"
#include "Data/CameraPresetDataAsset.h"

void UProjectHCameraSubsystem::PushCameraPreset(UCameraPresetDataAsset* Preset, int32 Priority)
{
	if (!Preset) return;

	FCameraStackEntry Entry;
	Entry.Preset = Preset;
	Entry.Priority = Priority;

	CameraStack.Add(Entry);
    
	// 우선순위가 높은 것이 마지막에 오도록 정렬
	CameraStack.Sort();
}

void UProjectHCameraSubsystem::PopCameraPreset(UCameraPresetDataAsset* Preset)
{
	if (!Preset) return;

	// 해당 프리셋을 찾아 제거
	for (int32 i = 0; i < CameraStack.Num(); ++i)
	{
		if (CameraStack[i].Preset == Preset)
		{
			CameraStack.RemoveAt(i);
			break;
		}
	}
}

void UProjectHCameraSubsystem::SetDefaultPreset(UCameraPresetDataAsset* InDefault)
{
	DefaultLevelPreset = InDefault;
}

UCameraPresetDataAsset* UProjectHCameraSubsystem::GetActivePreset() const
{
	// 1. 스택에 볼륨 데이터가 있다면 가장 마지막(최고 우선순위) 데이터 반환
	if (CameraStack.Num() > 0)
	{
		return CameraStack.Last().Preset;
	}

	// 2. 스택이 비어있다면 설정해둔 레벨 기본 프리셋 반환
	return DefaultLevelPreset;
}