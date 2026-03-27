#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

UENUM(BlueprintType)
enum class ECameraVolumeType : uint8
{
	Dynamic		UMETA(DisplayName = "Dynamic (추적)"),
	Static		UMETA(DisplayName = "Static (고정)")
};

UENUM(BlueprintType)
enum class ECameraProjectionType : uint8
{
	Perspective		UMETA(DisplayName = "Perspective (원근)"),
	Orthographic	UMETA(DisplayName = "Orthographic (직교)")
};

USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
	GENERATED_BODY()

	// ═══════════ 1. 카메라 기본 ═══════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본",
		meta=(DisplayName="볼륨 유형", ToolTip="Dynamic=플레이어 추적, Static=볼륨 고정"))
	ECameraVolumeType VolumeType = ECameraVolumeType::Dynamic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본",
		meta=(DisplayName="투영 모드", ToolTip="Perspective=원근, Orthographic=직교"))
	ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본",
		meta=(DisplayName="캐릭터 추적"))
	bool bFollowPawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본",
		meta=(DisplayName="폰 컨트롤 로테이션"))
	bool bUsePawnControlRotation = false;

	// ─── Dynamic ──────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 암 길이", EditCondition="VolumeType==ECameraVolumeType::Dynamic", ClampMin="0.0"))
	float TargetArmLength = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 회전값", EditCondition="VolumeType==ECameraVolumeType::Dynamic"))
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 중심점 오프셋", EditCondition="VolumeType==ECameraVolumeType::Dynamic"))
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="포커스 포인트 오프셋", EditCondition="VolumeType==ECameraVolumeType::Dynamic"))
	FVector FocusPointOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="시야각 (FOV)",
			  EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Perspective",
			  ClampMin="5.0", ClampMax="170.0"))
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Dynamic",
		meta=(DisplayName="직교 너비",
			  EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Orthographic",
			  ClampMin="100.0"))
	float OrthoWidth = 2048.0f;

	// ─── Static ───────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Static",
		meta=(DisplayName="고정 카메라 위치 오프셋", EditCondition="VolumeType==ECameraVolumeType::Static"))
	FVector StaticCameraOffset = FVector(0.f, 0.f, 800.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Static",
		meta=(DisplayName="고정 카메라 회전", EditCondition="VolumeType==ECameraVolumeType::Static"))
	FRotator StaticCameraRotation = FRotator(-60.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Static",
		meta=(DisplayName="시야각 (FOV)",
			  EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Perspective",
			  ClampMin="5.0", ClampMax="170.0"))
	float StaticFieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|1. 카메라 기본|Static",
		meta=(DisplayName="직교 너비",
			  EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Orthographic",
			  ClampMin="100.0"))
	float StaticOrthoWidth = 2048.0f;

	// ═══════════ 2. 전환 및 래그 ═══════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|2. 전환 및 래그",
		meta=(DisplayName="진입 전환 시간 (초)", ClampMin="0.0"))
	float BlendTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|2. 전환 및 래그",
		meta=(DisplayName="퇴장 전환 시간 (초)", ToolTip="-1=대상의 BlendTime 따름, 0=즉시", ClampMin="-1.0"))
	float ExitBlendTime = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|2. 전환 및 래그",
		meta=(DisplayName="추적 보간 속도", EditCondition="bFollowPawn", ClampMin="0.5", ClampMax="50.0"))
	float TrackingInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|2. 전환 및 래그",
		meta=(DisplayName="위치 래그 활성화"))
	bool bEnableLocationLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|2. 전환 및 래그",
		meta=(DisplayName="위치 래그 속도", EditCondition="bEnableLocationLag", ClampMin="0.5", ClampMax="50.0"))
	float LocationLagSpeed = 3.0f;

	// ═══════════ 3. 고급 설정 ═══════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(DisplayName="틸트 쉬프트 활성화"))
	bool bEnableTiltShift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="조리개 (F-Stop)"))
	float ApertureFStop = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="센서 폭 (mm)"))
	float SensorWidth = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="수동 초점 거리"))
	float ManualFocusDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="근거리 블러 세기"))
	float NearBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="원거리 블러 세기"))
	float FarBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 프리셋 설정|3. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="블러 경계 부드러움"))
	float FarTransitionRegion = 1000.f;

	// ─── 헬퍼 ─────────────────────────────
	float GetEffectiveFOV() const { return (VolumeType == ECameraVolumeType::Static) ? StaticFieldOfView : FieldOfView; }
	float GetEffectiveOrthoWidth() const { return (VolumeType == ECameraVolumeType::Static) ? StaticOrthoWidth : OrthoWidth; }
};


UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Camera Settings", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings Settings;
};