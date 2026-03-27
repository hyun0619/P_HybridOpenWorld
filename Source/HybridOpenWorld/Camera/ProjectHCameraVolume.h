#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "ProjectHCameraVolume.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(HideCategories=(Rendering, Replication, Input, Actor, LOD, Cooking))
class HYBRIDOPENWORLD_API AProjectHCameraVolume : public AActor
{
    GENERATED_BODY()

public:
    AProjectHCameraVolume();
    virtual void OnConstruction(const FTransform& Transform) override;

    FCameraPresetSettings GetCameraSettings() const { return LocalSettings; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
    FVector GetVolumeCenter() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
    FVector GetVolumeExtent() const;

    UFUNCTION(BlueprintCallable, Category="Camera Volume")
    void UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyPreviewPostProcessing();

    // ══════════════════════════════════════════
    // ★ 01. 에셋 관리 (따로 떨어져 나온 완전히 독립된 토글)
    // ══════════════════════════════════════════

    UPROPERTY(EditAnywhere, Category="01. 에셋 관리", meta=(DisplayName="연동할 DA 원본", DisplayPriority="1"))
    UCameraPresetDataAsset* LinkedDataAsset;

    UFUNCTION(CallInEditor, Category="01. 에셋 관리", meta=(DisplayName="DA 불러오기 (Read)", DisplayPriority="2"))
    void LoadFromDataAsset();

    UFUNCTION(CallInEditor, Category="01. 에셋 관리", meta=(DisplayName="DA에 저장하기 (Write)", DisplayPriority="3"))
    void SaveToDataAsset();

    UPROPERTY(EditAnywhere, Category="01. 에셋 관리", meta=(DisplayName="볼륨 우선순위 (Priority)", ToolTip="높을수록 우선 적용", DisplayPriority="4"))
    int32 Priority = 0;

    // ══════════════════════════════════════════
    // ★ 카메라 설정 (02, 03, 04, 05가 세부 카테고리로 묶이는 거대한 토글)
    // ══════════════════════════════════════════

    UPROPERTY(EditAnywhere, Category="카메라 설정", meta=(ShowOnlyInnerProperties))
    FCameraPresetSettings LocalSettings;

    // 05. 볼륨 레이아웃은 구조체 바깥에 있으므로 직접 소분류( | )를 지정해줍니다.
    UPROPERTY(EditAnywhere, Category="카메라 설정|05. 볼륨 레이아웃", meta=(DisplayName="볼륨 크기 (Half Extent)"))
    FVector VolumeExtent = FVector(500.f, 500.f, 200.f);

    // ══════════════════════════════════════════
    // 내부 컴포넌트
    // ══════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    USpringArmComponent* PreviewSpringArm;

    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    UCameraComponent* PreviewCamera;
};