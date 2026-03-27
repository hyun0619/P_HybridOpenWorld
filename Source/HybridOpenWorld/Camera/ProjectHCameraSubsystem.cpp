#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h"


void UProjectHCameraSubsystem::PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator)
{
	if (!Instigator) return;

	// 중복 방지: 같은 Instigator가 이미 스택에 있으면 갱신
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

		// ★ 핵심: 볼륨 액터라면 실시간 LocalSettings를 가져온다
		if (AProjectHCameraVolume* Volume = Cast<AProjectHCameraVolume>(ActiveInstigator))
		{
			OutSettings = Volume->GetCameraSettings();
			return true;
		}

		// 볼륨이 아닌 다른 액터가 요청했다면 스택에 저장된 값
		OutSettings = CameraStack.Last().Settings;
		return true;
	}

	// 기본 카메라 DA
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

// ──────────────────────────────────────────────────
// 신규 API
// ──────────────────────────────────────────────────

AProjectHCameraVolume* UProjectHCameraSubsystem::GetActiveVolume() const
{
	AActor* Instigator = GetActiveInstigator();
	return Instigator ? Cast<AProjectHCameraVolume>(Instigator) : nullptr;
}

void UProjectHCameraSubsystem::NotifyVolumeSettingsChanged(AProjectHCameraVolume* Volume)
{
	if (!Volume) return;

	// 해당 볼륨이 현재 활성 상태인 경우, 스택의 Settings도 갱신
	for (FCameraStackEntry& Entry : CameraStack)
	{
		if (Entry.Instigator == Volume)
		{
			Entry.Settings = Volume->GetCameraSettings();
			break;
		}
	}
	// CameraActor가 매 Tick마다 GetActivePreset을 호출하므로
	// 여기서 별도 알림 없이도 다음 프레임에 자동 반영됨
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