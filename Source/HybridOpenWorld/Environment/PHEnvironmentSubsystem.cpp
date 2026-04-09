#include "PHEnvironmentSubsystem.h"
#include "Game/PHGameInstance.h"

void UPHEnvironmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurrentTime = StartingTime; // 기본 설정값으로 시간 초기화
	
	// ★ GameInstance를 한 번만 캐싱 (기존: OnTimerUpdate에서 매번 Cast)
	CachedGameInstance = Cast<UPHGameInstance>(GetGameInstance());
	
	if (CachedGameInstance)
	{
		if (CachedGameInstance->PlayerData.SavedTime >= 0.0f)
		{
			CurrentTime = CachedGameInstance->PlayerData.SavedTime;
		}
		
		CachedGameInstance->GetTimerManager().SetTimer(
			TimerUpdateTimerHandle,
			this,
			&UPHEnvironmentSubsystem::OnTimerUpdate,
			TimeUpdateInterval,
			true);
	}
}

void UPHEnvironmentSubsystem::Deinitialize()
{
	// 서브시스템 종료 시 타이머를 명확히 해제하여 메모리 누수 방지
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->GetTimerManager().ClearTimer(TimerUpdateTimerHandle);
	}
	
	Super::Deinitialize();
}

void UPHEnvironmentSubsystem::OnTimerUpdate()
{
	// 시간이 흐르도록 하는 코드
	CurrentTime += TimeFlowSpeed;
	if (CurrentTime >= 24.0f) CurrentTime -= 24.0f; // ★ 0으로 초기화 대신 넘친 만큼 보존

	if (UPHGameInstance* GI = Cast<UPHGameInstance>(GetGameInstance()))
	{
		// ★ 캐싱된 포인터 사용 (매번 Cast 제거)
		if (CachedGameInstance)
		{
			CachedGameInstance->PlayerData.SavedTime = CurrentTime;
		}
	}
	
	//추후 프리셋 보간 로직 호출 예정
}

void UPHEnvironmentSubsystem::UpdateEnvironmentLerp()
{
}


