#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraSubsystem.generated.h"

class AProjectHCameraVolume;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveVolumeChanged,
	AProjectHCameraVolume*, NewVolume, AProjectHCameraVolume*, PreviousVolume);

UCLASS()
class HYBRIDOPENWORLD_API UProjectHCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ─── 기존 API ─────────────────────────────

	void PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator);
	void PopCameraPreset(AActor* Instigator);

	bool GetActivePreset(FCameraPresetSettings& OutSettings) const;
	AActor* GetActiveInstigator() const;
	void SetDefaultPreset(class UCameraPresetDataAsset* InDefaultDA);

	// ─── 신규 API ─────────────────────────────

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera System")
	AProjectHCameraVolume* GetActiveVolume() const;

	void NotifyVolumeSettingsChanged(AProjectHCameraVolume* Volume);

	/**
	 * 퇴장 블렌드 타임 오버라이드 가져오기.
	 * Pop 직후 한 번만 유효하고, 읽으면 리셋됩니다.
	 * @return true면 OutBlendTime에 오버라이드 값이 담김
	 */
	bool ConsumeExitBlendOverride(float& OutBlendTime);

	UPROPERTY(BlueprintAssignable, Category="Camera System|Events")
	FOnActiveVolumeChanged OnActiveVolumeChanged;

private:
	UPROPERTY()
	UCameraPresetDataAsset* DefaultLevelDA = nullptr;

	UPROPERTY()
	TArray<FCameraStackEntry> CameraStack;

	UPROPERTY()
	TWeakObjectPtr<AProjectHCameraVolume> CachedActiveVolume;

	/** Pop 시 저장되는 퇴장 블렌드 오버라이드 (-1 = 없음) */
	float PendingExitBlendOverride = -1.0f;

	void CheckAndBroadcastVolumeChange();
};