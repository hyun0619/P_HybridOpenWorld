#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"


/* 볼륨 타입 - 카메라 추적 or 고정 */
UENUM(BlueprintType)
enum class ECameraVolumeType : uint8
{
    Dynamic       UMETA(DisplayName = "Dynamic (추적)"),
    Static        UMETA(DisplayName = "Static (고정)")
};

/* 투영 모드 - 원근 or 직교 */
UENUM(BlueprintType)
enum class ECameraProjectionType : uint8
{
    Perspective    UMETA(DisplayName = "Perspective (원근)"),
    Orthographic   UMETA(DisplayName = "Orthographic (직교)")
};

/*
 * 카메라의 모든 설정값을 담는 데이터 컨테이너
 */
USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
    GENERATED_BODY()
    
    /* 1. 카메라 기본 설정 - 유형, 투영 모드, 추적 체크 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="볼륨 유형", ToolTip="Dynamic=플레이어 추적, Static=볼륨 중심 고정"))
    ECameraVolumeType VolumeType = ECameraVolumeType::Dynamic;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="투영 모드", ToolTip="Perspective=현실적 공간감(3D), Orthographic=2D 및 쿼터뷰(아이소메트릭)"))
    ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="캐릭터 추적", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides))
    bool bFollowPawn = true;
    
    /* 1. 카메라 기본 설정 - Dynamic : 암 길이, 회전값, 오프셋, FOV, 직교 너비 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 암 길이", ToolTip="카메라와 대상 사이 거리", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, ClampMin="0.0"))
    float TargetArmLength = 1000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 회전값", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides))
    FRotator Rotation = FRotator(-30.f, 0.f, 0.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 카메라 중심점 오프셋", ToolTip="카메라 중심점 상하좌우 이동", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides))
    FVector CameraOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 포커스 포인트 오프셋", ToolTip="폰 추적 시 위치에 더해짐(ex. 머리 위를 중심으로 잡고 싶을 때)", EditCondition="VolumeType==ECameraVolumeType::Dynamic", EditConditionHides))
    FVector FocusPointOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 시야각 (FOV)", ToolTip="값이 클수록 넓게 보임", EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Perspective", EditConditionHides, ClampMin="5.0", ClampMax="170.0"))
    float FieldOfView = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Dynamic] 직교 너비", ToolTip="값이 클수록 넓은 범위", EditCondition="VolumeType==ECameraVolumeType::Dynamic&&ProjectionType==ECameraProjectionType::Orthographic", EditConditionHides, ClampMin="100.0"))
    float OrthoWidth = 2038.0f;

    /* 1. 카메라 기본 설정 - Static : 카메라 위치, 회전값, FOV, 직교 너비 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Static] 카메라 위치 오프셋", ToolTip="카메라를 놓을 위치", EditCondition="VolumeType==ECameraVolumeType::Static", EditConditionHides))
    FVector StaticCameraOffset = FVector(0.f, 0.f, 800.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Static] 카메라 회전값", EditCondition="VolumeType==ECameraVolumeType::Static", EditConditionHides))
    FRotator StaticCameraRotation = FRotator(-60.f, 0.f, 0.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Static] 시야각 (FOV)", ToolTip="값이 클수록 넓게 보임", EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Perspective", EditConditionHides, ClampMin="5.0", ClampMax="170.0"))
    float StaticFieldOfView = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|01. 카메라 기본", meta=(DisplayName="[Static] 직교 너비", ToolTip="값이 클수록 넓게 보임", EditCondition="VolumeType==ECameraVolumeType::Static&&ProjectionType==ECameraProjectionType::Orthographic", EditConditionHides, ClampMin="100.0"))
    float StaticOrthoWidth = 2038.0f;

    /* 2. 전환 및 레그 - 진입 및 회전 전환 시간, 추적 보간 속도, 위치 레그 활성화 및 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 전환 및 래그", meta=(DisplayName="진입 전환 시간 (초)", ToolTip="볼륨 진입 시 전환 시간", ClampMin="0.0"))
    float BlendTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 전환 및 래그", meta=(DisplayName="퇴장 전환 시간 (초)", ToolTip="볼륨 나갈 때 전환 시간/-1=대상의 BlendTime 따름, 0=즉시", ClampMin="-1.0"))
    float ExitBlendTime = -1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 전환 및 래그", meta=(DisplayName="추적 보간 속도", ToolTip="카메라가 폰을 따라가는 보간 속도, 높을수록 즉각 반응", EditCondition="bFollowPawn&&VolumeType==ECameraVolumeType::Dynamic", EditConditionHides, ClampMin="0.5", ClampMax="50.0"))
    float TrackingInterpSpeed = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 전환 및 래그", meta=(DisplayName="위치 래그 활성화", ToolTip="카메라 이동 시 약간 뒤처지는 느낌", DisplayPriority="33"))
    bool bEnableLocationLag = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|02. 전환 및 래그", meta=(DisplayName="위치 래그 속도", ToolTip="높을수록 빠르게 따라붙음", EditCondition="bEnableLocationLag", EditConditionHides, ClampMin="0.5", ClampMax="50.0"))
    float LocationLagSpeed = 3.0f;

    /* 3. 포스트 프로세스 - 틸트 쉬프트 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(DisplayName="틸트 쉬프트 활성화"))
    bool bEnableTiltShift = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="조리개 (F-Stop)", ToolTip="낮을수록 배경 흐려짐"))
    float ApertureFStop = 0.8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="센서 폭 (mm)", ToolTip="클수록 피사계심도 범위 넓어짐"))
    float SensorWidth = 144.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="수동 초점 거리", ToolTip="초점이 맞는 거리, 해당 거리의 물체 선명해짐"))
    float ManualFocusDistance = 2000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="근거리 블러 세기", ToolTip="초점 거리보다 가까운 물체의 블러 세기"))
    float NearBlurRadius = 15.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="원거리 블러 세기", ToolTip="초점 거리보다 먼 물체의 블러 세기"))
    float FarBlurRadius = 15.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="카메라 설정|03. 포스트 프로세스", meta=(EditCondition="bEnableTiltShift", EditConditionHides, DisplayName="블러 경계 부드러움", ToolTip="선명~블러 영역 경계의 부드러움 정도, 클수록 흐려짐"))
    float FarTransitionRegion = 1000.f;


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