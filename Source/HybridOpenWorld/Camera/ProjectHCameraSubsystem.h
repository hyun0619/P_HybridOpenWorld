#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraSubsystem.generated.h"

class AProjectHCameraVolume;

/*
 * 현재 활성화된 카메라 설정 관리를 위한 스택 개별 요소
 */
USTRUCT()
struct FCameraStackEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FCameraPresetSettings Settings; // 카메라 상세 설정값
	UPROPERTY()
	int32 Priority = 0; // 우선순위
	UPROPERTY()
	AActor* Instigator = nullptr; // 이 설정을 요청한 주체 - 보통 CameraVolume Actor

	bool operator<(const FCameraStackEntry& Other) const // 정렬을 위한 연산자 오버로딩 - Priority 기준
	{
		return Priority < Other.Priority;
	}
};

// 카메라 볼륨이 변경되었을 때 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveVolumeChanged, AProjectHCameraVolume*, NewVolume,
	AProjectHCameraVolume*, PreviousVolume);

/*
 * 월드 내 모든 카메라 볼륨 요청 수집, 최종 적용될 카메라 값 계산
 */
UCLASS()
class HYBRIDOPENWORLD_API UProjectHCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 기존 API */
	// 새로운 카메라 설정 스택 추가 - 볼륨 진입 시 호출
	void PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator);
	void PopCameraPreset(AActor* Instigator); // 특정 주체의 설정을 스택에서 제거 - 볼륨 퇴장 시 호출
	bool GetActivePreset(FCameraPresetSettings& OutSettings) const; // 현재 가장 우선순위 높은 설정값 가져오기
	AActor* GetActiveInstigator() const; // 현재 카메라 제어 액터 가져오기
	void SetDefaultPreset(class UCameraPresetDataAsset* InDefaultDA); // 기본 카메라 설정 지정 - 볼륨 없는 곳
	
	/* 신규 API */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera System")
	AProjectHCameraVolume* GetActiveVolume() const; // 현재 활성화된 카메라 볼륨 액터 반환
	
	void NotifyVolumeSettingsChanged(AProjectHCameraVolume* Volume); // 볼륨 내부 수치가 실시간으로 변경됨을 알림
	
	/**
	 * 퇴장 시의 블렌드 타임(전환 시간) 값을 한 번만 가져옴
	 * return true면 OutBlendTime에 설정된 값이 담기고 내부 변수는 리셋
	 */
	bool ConsumeExitBlendOverride(float& OutBlendTime);

	UPROPERTY(BlueprintAssignable, Category="Camera System|Events")
	FOnActiveVolumeChanged OnActiveVolumeChanged; // 카메라 볼륨 바뀌었을 때 발생하는 이벤트

private:
	UPROPERTY()
	UCameraPresetDataAsset* DefaultLevelDA = nullptr; // 월드 기본 카메라 설정 DA
	UPROPERTY()
	TArray<FCameraStackEntry> CameraStack; // 현재 겹쳐있는 모든 카메라 설정들의 목록
	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraVolume> CachedActiveVolume; // 이전 볼륨 기억 -> 변경 사항 감지 변수
	
	float PendingExitBlendOverride = -1.0f; // 볼륨 나갈 때 적용할 임시 블렌드 시간

	void CheckAndBroadcastVolumeChange(); // 전체 활성 볼륨 바뀌었는지 체크 -> 이벤트 방송하는 함수
};