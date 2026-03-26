#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

// ★ 1. 카메라 설정 데이터 구조체
USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
    GENERATED_BODY()

    // --- [카메라 렌즈 및 위치] ---
    UPROPERTY(EditAnywhere, Category="00. Camera Settings", meta=(DisplayName="카메라 암 길이"))
    float TargetArmLength = 1000.f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="시야각 (FOV)"))
    float FieldOfView = 90.0f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="고정 회전값"))
    FRotator Rotation = FRotator(-30.f, 0.f, 0.f);
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="카메라 중심점 오프셋"))
    FVector CameraOffset = FVector::ZeroVector;
    
    // --- [이동 및 추적] ---
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="캐릭터 추적 여부"))
    bool bFollowPawn = true;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="캐릭터를 따라가는 속도", EditCondition = "bFollowPawn"))
    float TrackingInterpSpeed = 5.0f;
    UPROPERTY(EditAnywhere, Category="00. Camera Settings", meta=(DisplayName="카메라 전환 시간"))
    float BlendTime = 2.0f;
    
    // --- [포스트 프로세스 (틸트 쉬프트)] ---
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="틸트 쉬프트 활성화"))
    bool bEnableTiltShift = false;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="조리개 (F-Stop)"))
    float ApertureFStop = 0.8f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="센서 폭 (mm)"))
    float SensorWidth = 144.0f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="수동 초점 거리"))
    float ManualFocusDistance = 2000.f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 가까운 블러 세기"))
    float NearBlurRadius = 15.f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 먼 블러 세기"))
    float FarBlurRadius = 15.f;
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(EditCondition="bEnableTiltShift", DisplayName="블러 경계 부드러움 세기"))
    float FarTransitionRegion = 1000.f;
};

// ★ 2. 레벨 기본 뷰 세팅용 데이터 에셋
UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, Category="Camera Settings", meta=(ShowOnlyInnerProperties))
    FCameraPresetSettings Settings;
};