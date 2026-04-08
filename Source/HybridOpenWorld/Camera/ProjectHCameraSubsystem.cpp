#include "ProjectHCameraSubsystem.h"
#include "ProjectHCameraVolume.h"

/* 새로운 카메라 설정을 스택에 넣고 우선순위 정렬 */
void UProjectHCameraSubsystem::PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator)
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
void UProjectHCameraSubsystem::PopCameraPreset(AActor* Instigator)
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
bool UProjectHCameraSubsystem::GetActivePreset(FCameraPresetSettings& OutSettings) const
{
	if (CameraStack.IsEmpty())
	{
		if (DefaultLevelDA == nullptr)
			return false;

		OutSettings = DefaultLevelDA->Settings;
		return true;
	}
	
	AActor* ActiveInstigator = CameraStack.Last().Instigator;
	// 볼륨 액터라면 최신 데이터 다시 가져옴 - 실시간 수정 반영
	if (AProjectHCameraVolume* Volume = Cast<AProjectHCameraVolume>(ActiveInstigator))
	{
		OutSettings = Volume->GetCameraSettings();
		return true;
	}
	
	OutSettings = CameraStack.Last().Settings;
	return true;
}

/* 현재 활성화된 제어 주체 반환 */
AActor* UProjectHCameraSubsystem::GetActiveInstigator() const
{
	if (CameraStack.IsEmpty())
	{
		return nullptr;
	}
	
	return CameraStack.Last().Instigator;
}

/* 레벨 기본 설정값 지정 */
void UProjectHCameraSubsystem::SetDefaultPreset(UCameraPresetDataAsset* InDefaultDA)
{
	DefaultLevelDA = InDefaultDA;
}

/* 현재 활성화된 볼륨 액터 타입으로 가져오기*/
AProjectHCameraVolume* UProjectHCameraSubsystem::GetActiveVolume() const
{
	AActor* Instigator = GetActiveInstigator();
	return Cast<AProjectHCameraVolume>(Instigator);
}

/* 볼륨 내 수치가 에디터나 런타임에서 변경되었을 때 스택 데이터 동기화 */
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

/* 퇴장 블렌드 시간 소비 - 한번 읽으면 초기화되는 데이터 */
bool UProjectHCameraSubsystem::ConsumeExitBlendOverride(float& OutBlendTime)
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
void UProjectHCameraSubsystem::CheckAndBroadcastVolumeChange()
{
	AProjectHCameraVolume* CurrentVolume = GetActiveVolume();
	AProjectHCameraVolume* PrevVolume = CachedActiveVolume.Get();

	// 현재 활성화된 볼륨이 이전과 다르면 델리게이트를 통해 외부에 알림
	if (CurrentVolume != PrevVolume)
	{
		CachedActiveVolume = CurrentVolume;
		OnActiveVolumeChanged.Broadcast(CurrentVolume, PrevVolume);
	}
}