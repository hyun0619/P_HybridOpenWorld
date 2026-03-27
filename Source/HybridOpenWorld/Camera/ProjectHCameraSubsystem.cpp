#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h"


void UProjectHCameraSubsystem::PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator)
{
	if (!Instigator) return;

	// ★ 중복 방지: 같은 Instigator가 이미 있으면 갱신
	for (FCameraStackEntry& Entry : CameraStack)
	{
		if (Entry.Instigator == Instigator)
		{
			Entry.Settings = Settings;
			Entry.Priority = Priority;
			CameraStack.Sort();
			CheckAndBroadcastVolumeChange();
			return;
		}
	}

	FCameraStackEntry Entry;
	Entry.Settings = Settings;
	Entry.Priority = Priority;
	Entry.Instigator = Instigator;

	CameraStack.Add(Entry);
	CameraStack.Sort();

	CheckAndBroadcastVolumeChange();
}

void UProjectHCameraSubsystem::PopCameraPreset(AActor* Instigator)
{
	if (!Instigator) return;

	for (int32 i = 0; i < CameraStack.Num(); ++i)
	{
		if (CameraStack[i].Instigator == Instigator)
		{
			// ★ 퇴장 블렌드 오버라이드 저장
			// ExitBlendTime >= 0 이면 다음 전환에 이 값을 사용
			const float ExitBT = CameraStack[i].Settings.ExitBlendTime;
			if (ExitBT >= 0.0f)
			{
				PendingExitBlendOverride = ExitBT;
			}

			CameraStack.RemoveAt(i);
			break;
		}
	}

	CheckAndBroadcastVolumeChange();
}

bool UProjectHCameraSubsystem::GetActivePreset(FCameraPresetSettings& OutSettings) const
{
	if (CameraStack.Num() > 0)
	{
		AActor* ActiveInstigator = CameraStack.Last().Instigator;

		if (AProjectHCameraVolume* Volume = Cast<AProjectHCameraVolume>(ActiveInstigator))
		{
			OutSettings = Volume->GetCameraSettings();
			return true;
		}

		OutSettings = CameraStack.Last().Settings;
		return true;
	}

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

AProjectHCameraVolume* UProjectHCameraSubsystem::GetActiveVolume() const
{
	AActor* Instigator = GetActiveInstigator();
	return Instigator ? Cast<AProjectHCameraVolume>(Instigator) : nullptr;
}

void UProjectHCameraSubsystem::NotifyVolumeSettingsChanged(AProjectHCameraVolume* Volume)
{
	if (!Volume) return;
	for (FCameraStackEntry& Entry : CameraStack)
	{
		if (Entry.Instigator == Volume)
		{
			Entry.Settings = Volume->GetCameraSettings();
			break;
		}
	}
}

bool UProjectHCameraSubsystem::ConsumeExitBlendOverride(float& OutBlendTime)
{
	if (PendingExitBlendOverride >= 0.0f)
	{
		OutBlendTime = PendingExitBlendOverride;
		PendingExitBlendOverride = -1.0f; // 한 번 읽으면 리셋
		return true;
	}
	return false;
}

void UProjectHCameraSubsystem::CheckAndBroadcastVolumeChange()
{
	AProjectHCameraVolume* CurrentVolume = GetActiveVolume();
	AProjectHCameraVolume* PrevVolume = CachedActiveVolume.Get();

	if (CurrentVolume != PrevVolume)
	{
		CachedActiveVolume = CurrentVolume;
		OnActiveVolumeChanged.Broadcast(CurrentVolume, PrevVolume);
	}
}