#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h" // ★ 볼륨 액터를 캐싱하기 위해 인클루드


void UProjectHCameraSubsystem::PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator)
{
	if (!Instigator) return;

	FCameraStackEntry Entry;
	Entry.Settings = Settings;
	Entry.Priority = Priority;
	Entry.Instigator = Instigator;

	CameraStack.Add(Entry);
	CameraStack.Sort();
}

void UProjectHCameraSubsystem::PopCameraPreset(AActor* Instigator)
{
	if (!Instigator) return;

	for (int32 i = 0; i < CameraStack.Num(); ++i)
	{
		if (CameraStack[i].Instigator == Instigator)
		{
			CameraStack.RemoveAt(i);
			break;
		}
	}
}

bool UProjectHCameraSubsystem::GetActivePreset(FCameraPresetSettings& OutSettings) const
{
	if (CameraStack.Num() > 0)
	{
		AActor* ActiveInstigator = CameraStack.Last().Instigator;
        
		// ★ 핵심: 현재 카메라를 조종하는 게 볼륨 액터라면?
		if (AProjectHCameraVolume* Volume = Cast<AProjectHCameraVolume>(ActiveInstigator))
		{
			// 볼륨의 디테일 창에 있는 '실시간' 세팅을 즉시 가져옵니다!
			OutSettings = Volume->GetCameraSettings();
			return true;
		}
        
		// 볼륨이 아닌 다른 액터가 요청했다면 스택에 저장된 값을 줍니다.
		OutSettings = CameraStack.Last().Settings;
		return true;
	}
    
	// ★ 기본 카메라도 DA에서 실시간으로 가져옵니다!
	if (DefaultLevelDA)
	{
		OutSettings = DefaultLevelDA->Settings;
		return true;
	}

	return false;
}

AActor* UProjectHCameraSubsystem::GetActiveInstigator() const
{
	if (CameraStack.Num() > 0) return CameraStack.Last().Instigator;
	return nullptr;
}

void UProjectHCameraSubsystem::SetDefaultPreset(UCameraPresetDataAsset* InDefaultDA)
{
	DefaultLevelDA = InDefaultDA;
}