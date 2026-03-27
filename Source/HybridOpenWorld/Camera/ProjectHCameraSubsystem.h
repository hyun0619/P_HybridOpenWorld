#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraSubsystem.generated.h"

class AProjectHCameraVolume;

// ──────────────────────────────────────────────
// 스택 엔트리 (우선순위 정렬)
// ──────────────────────────────────────────────

USTRUCT()
struct FCameraStackEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FCameraPresetSettings Settings;

	UPROPERTY()
	int32 Priority = 0;

	UPROPERTY()
	AActor* Instigator = nullptr;

	bool operator<(const FCameraStackEntry& Other) const
	{
		return Priority < Other.Priority;
	}
};

// ──────────────────────────────────────────────
// 카메라 볼륨 전환 이벤트
// ──────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveVolumeChanged,
	AProjectHCameraVolume*, NewVolume, AProjectHCameraVolume*, PreviousVolume);


UCLASS()
class HYBRIDOPENWORLD_API UProjectHCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ─── 기존 API (유지) ──────────────────────

	void PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator);
	void PopCameraPreset(AActor* Instigator);

	bool GetActivePreset(FCameraPresetSettings& OutSettings) const;
	AActor* GetActiveInstigator() const;
	void SetDefaultPreset(class UCameraPresetDataAsset* InDefaultDA);

	// ─── 신규 API ─────────────────────────────

	/** 현재 활성 볼륨 액터 반환 (볼륨이 아닌 Instigator이면 nullptr) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera System")
	AProjectHCameraVolume* GetActiveVolume() const;

	/** 볼륨의 런타임 설정 변경 시 호출 (활성 볼륨이면 즉시 반영) */
	void NotifyVolumeSettingsChanged(AProjectHCameraVolume* Volume);

	/** 볼륨 변경 이벤트 (블루프린트 바인딩 가능) */
	UPROPERTY(BlueprintAssignable, Category="Camera System|Events")
	FOnActiveVolumeChanged OnActiveVolumeChanged;

private:
	UPROPERTY()
	UCameraPresetDataAsset* DefaultLevelDA = nullptr;

	UPROPERTY()
	TArray<FCameraStackEntry> CameraStack;

	/** 이전 활성 볼륨 캐시 (변경 이벤트 발생용) */
	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraVolume> CachedActiveVolume;

	/** 활성 볼륨 변경 감지 및 이벤트 브로드캐스트 */
	void CheckAndBroadcastVolumeChange();
};