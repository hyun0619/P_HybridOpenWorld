#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/CameraPresetDataAsset.h" // 구조체를 알기 위해 인클루드
#include "ProjectHCameraSubsystem.generated.h"

USTRUCT()
struct FCameraStackEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FCameraPresetSettings Settings; // 구조체 자체를 저장

	UPROPERTY()
	int32 Priority = 0;

	UPROPERTY()
	AActor* Instigator = nullptr; 

	bool operator<(const FCameraStackEntry& Other) const 
	{ 
		return Priority < Other.Priority; 
	}
};

UCLASS()
class HYBRIDOPENWORLD_API UProjectHCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void PushCameraPreset(const FCameraPresetSettings& Settings, int32 Priority, AActor* Instigator);
	void PopCameraPreset(AActor* Instigator); // 뺄 때는 볼륨만 알려주면 됩니다!

	bool GetActivePreset(FCameraPresetSettings& OutSettings) const;
	AActor* GetActiveInstigator() const; 
	// ★ 수정: 구조체 대신 DA 포인터 원본을 받습니다.
	void SetDefaultPreset(class UCameraPresetDataAsset* InDefaultDA);

private:
	// ★ 수정: 기본 프리셋도 원본을 들고 있게 합니다.
	UPROPERTY()
	UCameraPresetDataAsset* DefaultLevelDA = nullptr;

	UPROPERTY()
	TArray<FCameraStackEntry> CameraStack;
};