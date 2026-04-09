#include "PHCameraSubsystem.h"
#include "PHCameraVolume.h"

/* 새로운 카메라 설정을 스택에 넣고 우선순위 정렬 */
void UPHCameraSubsystem::PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator)
{
	if (!Instigator) return;

	// 같은 액터가 스택에 있다면 값만 갱신
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

	// 새로운 항목 추가
	FCameraStackEntry Entry;
	Entry.Settings = Settings;
	Entry.Priority = Priority;
	Entry.Instigator = Instigator;

	// 우선 순위 정렬
	CameraStack.Add(Entry);
	CameraStack.Sort();
	
	CheckAndBroadcastVolumeChange(); // 상태 변경 확인
}

/* 볼륨에서 나갈 때 해당 설정을 스택에서 제거 */
void UPHCameraSubsystem::PopCameraPreset(AActor* Instigator)
{
	if (!Instigator) return;

	for (int32 i = 0; i < CameraStack.Num(); ++i)
	{
		if (CameraStack[i].Instigator == Instigator)
		{
			// 볼륨을 나가는 순간의 부드러운 전환 시간 저장
			const float ExitBT = CameraStack[i].Settings.ExitBlendTime;
			if (ExitBT >= 0.0f)
			{
				ExitBlendQueue.Add(ExitBT);
			}
			CameraStack.RemoveAt(i);
			break;	
		}
	}
	CheckAndBroadcastVolumeChange(); // 제거 후 다시 상태 확인 - 다음 우선순위 볼륨으로 카메라 넘어감
}

/* 최종적으로 적용되어야 할 카메라 프리셋 결정 */
bool UPHCameraSubsystem::GetActivePreset(FCameraPresetSettings& OutSettings) const
{
	if (CameraStack.IsEmpty())
	{
		if (DefaultLevelDA == nullptr)
			return false;

		OutSettings = DefaultLevelDA->Settings;
		return true;
	}
	
	// 가장 우선순위가 높고 유효한 액터를 스택 최상단부터 찾음
	for (int32 i = CameraStack.Num() - 1; i >= 0; --i)
	{
		if (CameraStack[i].Instigator.IsValid())
		{
			AActor* ActiveInstigator = CameraStack[i].Instigator.Get();
			if (APHCameraVolume* Volume = Cast<APHCameraVolume>(ActiveInstigator))
			{
				OutSettings = Volume->GetCameraSettings();
				return true;
			}
			OutSettings = CameraStack[i].Settings;
			return true;
		}
	}
    
	// 모든 스택 데이터가 유효하지 않으면 기본값 반환
	if (DefaultLevelDA == nullptr) return false;
	OutSettings = DefaultLevelDA->Settings;
	return true;
}

/* 현재 활성화된 제어 주체 반환 */
AActor* UPHCameraSubsystem::GetActiveInstigator() const
{
	// 유효한 제어 주체만 반환
	for (int32 i = CameraStack.Num() - 1; i >= 0; --i)
	{
		if (CameraStack[i].Instigator.IsValid())
		{
			return CameraStack[i].Instigator.Get();
		}
	}
	return nullptr;
}

/* 레벨 기본 설정값 지정 */
void UPHCameraSubsystem::SetDefaultPreset(UCameraPresetDataAsset* InDefaultDA)
{
	DefaultLevelDA = InDefaultDA;
}

/* 현재 활성화된 볼륨 액터 타입으로 가져오기*/
APHCameraVolume* UPHCameraSubsystem::GetActiveVolume() const
{
	AActor* Instigator = GetActiveInstigator();
	return Cast<APHCameraVolume>(Instigator);
}

/* 볼륨 내 수치가 에디터나 런타임에서 변경되었을 때 스택 데이터 동기화 */
void UPHCameraSubsystem::NotifyVolumeSettingsChanged(APHCameraVolume* Volume)
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

/* 퇴장 블렌드 시간 소비 - 한번 읽으면 초기화되는 데이터 */
bool UPHCameraSubsystem::ConsumeExitBlendOverride(float& OutBlendTime)
{
	if (!ExitBlendQueue.IsEmpty())
	{
		// 가장 먼저 들어온 데이터 추출 - FIFO
		OutBlendTime = ExitBlendQueue[0];
		ExitBlendQueue.RemoveAt(0); // 첫 번째 요소 삭제
		return true;
	}
	return false;
}

/* 볼륨이 교체되었는지 확인, 이벤트 발생 */
void UPHCameraSubsystem::CheckAndBroadcastVolumeChange()
{
	// 상태 검사 시점 - 게임에서 파괴되어 무효화된 약참조 포인터들을 스택에서 청소
	CameraStack.RemoveAll([](const FCameraStackEntry& Entry) { return !Entry.Instigator.IsValid(); });
	
	APHCameraVolume* CurrentVolume = GetActiveVolume();
	APHCameraVolume* PrevVolume = CachedActiveVolume.Get();

	// 현재 활성화된 볼륨이 이전과 다르면 델리게이트를 통해 외부에 알림
	if (CurrentVolume != PrevVolume)
	{
		CachedActiveVolume = CurrentVolume;
		OnActiveVolumeChanged.Broadcast(CurrentVolume, PrevVolume);
	}
}