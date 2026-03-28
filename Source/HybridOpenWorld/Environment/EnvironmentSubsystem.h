#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EnvironmentSubsystem.generated.h"


class UEnvPresetDataAsset;
class UProjectHGameInstance;
class UEnvPresetDataAsset;
/**
 * 시간의 흐름과 날씨 변화 계산하는 환경 관리 서브시스템
 */
UCLASS()
class HYBRIDOPENWORLD_API UEnvironmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override; // 서브시스템 생성 시 실행
	virtual void Deinitialize() override; // 서브시스템 파괴 시 실행
	
	// 게임 시간을 수동으로 설정하거나 가져오는 함수
	UFUNCTION(BlueprintCallable, Category="Environment")
	void SetCurrentTime(float NewTime) {CurrentTime = NewTime;}
	UFUNCTION(BlueprintPure, Category="Environment")
	float GetCurrentTime() const {return CurrentTime;}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Environment|Settings")
	float TimeUpdateInterval = 0.1f; // 타이머 실행 간격 (0.1초 = 초당 10번 연산)
	UPROPERTY(EditDefaultsOnly, Category = "Environment|Settings")
	float TimeFlowSpeed = 0.01f; // 시간이 흐르는 속도
	UPROPERTY(EditDefaultsOnly, Category = "Environment|Settings")
	float StartingTime = 9.0f; // 게임 시작 시 기본 시작 시간 (오전 9시)
	
	UPROPERTY(EditDefaultsOnly, Category = "Environment|Presets")
	TArray<UEnvPresetDataAsset*> EnvironmentPresets; // 시간대별 프리셋들을 리스트로 관리
	
private:
	void OnTimerUpdate(); // 주기적으로 실행하여 환경 보간 등 처리
	FTimerHandle TimerUpdateTimerHandle;
	
	void UpdateEnvironmentLerp(); // 두 개의 프리셋 - 현재 프리셋, 다음 프리셋을 찾는 함수
	
	UPROPERTY()
	float CurrentTime; // 흐르는 시간을 저장하는 변수 (현재 시간)
	
	/** ★ 매 타이머마다 Cast하지 않도록 캐싱 */
	UPROPERTY()
	UProjectHGameInstance* CachedGameInstance = nullptr;
};