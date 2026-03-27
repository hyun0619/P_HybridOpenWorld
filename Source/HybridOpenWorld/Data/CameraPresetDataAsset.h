#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

UENUM(BlueprintType)
enum class ECameraVolumeType : uint8
{
    Dynamic       UMETA(DisplayName = "Dynamic (추적)"),
    Static        UMETA(DisplayName = "Static (고정)")
};

UENUM(BlueprintType)
enum class ECameraProjectionType : uint8
{
    Perspective    UMETA(DisplayName = "Perspective (원근)"),
    Orthographic   UMETA(DisplayName = "Orthographic (직교)")
};

USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
    GENERATED_BODY()

    // ═══════════ 02. 카메라 기본 ═══════════
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="볼륨 유형", ToolTip="Dynamic=플레이어 추적, Static=볼륨 고정", DisplayPriority="10"))
    ECameraVolumeType VolumeType = ECameraVolumeType::Dynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="투영 모드", DisplayPriority="11"))
    ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;

    // Static 상태일 때는 캐릭터 추적 옵션이 의미 없으므로 숨김 처리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="캐릭터 추적", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, DisplayPriority="12"))
    bool bFollowPawn = true;

    // [삭제 완료] 폰 컨트롤 로테이션 적용 기능 제거

    // ─── Dynamic 전용 ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 암 길이", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, ClampMin="0.0", DisplayPriority="14"))
    float TargetArmLength = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 회전값", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, DisplayPriority="15"))
    FRotator Rotation = FRotator(-30.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 중심점 오프셋", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, DisplayPriority="16"))
    FVector CameraOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 포커스 포인트 오프셋", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, DisplayPriority="17"))
    FVector FocusPointOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 시야각 (FOV)", EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Perspective", EditConditionHides, ClampMin="5.0", ClampMax="170.0", DisplayPriority="18"))
    float FieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Dynamic] 직교 너비", EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Orthographic", EditConditionHides, ClampMin="100.0", DisplayPriority="19"))
    float OrthoWidth = 2048.0f;

    // ─── Static 전용 ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Static] 카메라 위치 오프셋", EditCondition="VolumeType==ECameraVolumeType::Static", EditConditionHides, DisplayPriority="20"))
    FVector StaticCameraOffset = FVector(0.f, 0.f, 800.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Static] 카메라 회전값", EditCondition="VolumeType==ECameraVolumeType::Static", EditConditionHides, DisplayPriority="21"))
    FRotator StaticCameraRotation = FRotator(-60.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Static] 시야각 (FOV)", EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Perspective", EditConditionHides, ClampMin="5.0", ClampMax="170.0", DisplayPriority="22"))
    float StaticFieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 카메라 기본", meta=(DisplayName="[Static] 직교 너비", EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Orthographic", EditConditionHides, ClampMin="100.0", DisplayPriority="23"))
    float StaticOrthoWidth = 2048.0f;

    // ═══════════ 03. 전환 및 래그 ═══════════
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 전환 및 래그", meta=(DisplayName="진입 전환 시간 (초)", ClampMin="0.0", DisplayPriority="30"))
    float BlendTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 전환 및 래그", meta=(DisplayName="퇴장 전환 시간 (초)", ToolTip="-1=대상의 BlendTime 따름, 0=즉시", ClampMin="-1.0", DisplayPriority="31"))
    float ExitBlendTime = -1.0f;

    // 추적 보간 속도는 '캐릭터 추적'이 켜져있고, 'Dynamic'일 때만 보입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 전환 및 래그", meta=(DisplayName="추적 보간 속도", EditCondition="bFollowPawn&&VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, ClampMin="0.5", ClampMax="50.0", DisplayPriority="32"))
    float TrackingInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 전환 및 래그", meta=(DisplayName="위치 래그 활성화", DisplayPriority="33"))
    bool bEnableLocationLag = true;

    // 래그 속도는 래그가 활성화되어 있을 때만 보입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 전환 및 래그", meta=(DisplayName="위치 래그 속도", EditCondition="bEnableLocationLag", EditConditionHides, ClampMin="0.5", ClampMax="50.0", DisplayPriority="34"))
    float LocationLagSpeed = 3.0f;

    // ═══════════ 04. 고급 설정 (포스트 프로세스) ═══════════
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(DisplayName="틸트 쉬프트 활성화", DisplayPriority="40"))
    bool bEnableTiltShift = false;

    // 아래 포스트 프로세스 세부 항목들은 틸트 쉬프트가 켜졌을 때만 쫙 펼쳐집니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 조리개 (F-Stop)", DisplayPriority="41"))
    float ApertureFStop = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 센서 폭 (mm)", DisplayPriority="42"))
    float SensorWidth = 144.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 수동 초점 거리", DisplayPriority="43"))
    float ManualFocusDistance = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 근거리 블러 세기", DisplayPriority="44"))
    float NearBlurRadius = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 원거리 블러 세기", DisplayPriority="45"))
    float FarBlurRadius = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|04. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="└ 블러 경계 부드러움", DisplayPriority="46"))
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
    UPROPERTY(EditAnywhere, Category="카메라 설정", meta=(ShowOnlyInnerProperties))
    FCameraPresetSettings Settings;
};