#include "EnvironmentSubsystem.h"
#include "Game/ProjectHGameInstance.h"

void UEnvironmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurrentTime = StartingTime; // 기본 설정값으로 시간 초기화
	
	// 서브시스템 초기화 시점에 World가 아직 생성되지 않았을 경우를 대비한 안전 장치
	if (UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(GetGameInstance()))
	{
		if (GI->PlayerData.SavedTime >= 0.0f)
		{
			CurrentTime = GI->PlayerData.SavedTime;
		}
		
		GI->GetTimerManager().SetTimer( // Tick 의존 X, 전용 타이머 사용하여 CPU 점유율 고정
			TimerUpdateTimerHandle,
			this,
			&UEnvironmentSubsystem::OnTimerUpdate,
			TimeUpdateInterval,
			true);
	}
}

void UEnvironmentSubsystem::Deinitialize()
{
	// 서브시스템 종료 시 타이머를 명확히 해제하여 메모리 누수 방지
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->GetTimerManager().ClearTimer(TimerUpdateTimerHandle);
	}
	
	Super::Deinitialize();
}

void UEnvironmentSubsystem::OnTimerUpdate()
{
	// 시간이 흐르도록 하는 코드
	CurrentTime += TimeFlowSpeed;
	if (CurrentTime >= 24.0f) CurrentTime = 0.0f;

	if (UProjectHGameInstance* GI = Cast<UProjectHGameInstance>(GetGameInstance()))
	{
		// 실시간으로 변하는 시간을 GameInstance에 계속 동기화
		GI->PlayerData.SavedTime = CurrentTime;
	}
	
	//추후 프리셋 보간 로직 호출 예정
}

void UEnvironmentSubsystem::UpdateEnvironmentLerp()
{
}


