#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CameraPresetDataAsset.h"
#include "PHCameraVolume.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;

/*
 * 레벨에 배치하는 카메라 볼륨 박스 트리거 액터 - 영역 진입 시 카메라 연출 변경
 */
UCLASS(HideCategories=(Rendering, Replication, Input, Actor, LOD, Cooking))
class HYBRIDOPENWORLD_API APHCameraVolume : public AActor
{
    GENERATED_BODY()

public:
    APHCameraVolume();
	
    virtual void OnConstruction(const FTransform& Transform) override; // 액터 수치 변경마다 실시간 호출(프리뷰 업데이트용)

    FCameraPresetSettings GetCameraSettings() const; // 현재 볼륨이 가진 설정값 반환

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
    FVector GetVolumeCenter() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Camera Volume")
    FVector GetVolumeExtent() const; 
    UFUNCTION(BlueprintCallable, Category="Camera Volume")
	
    void UpdateSettingsAtRuntime(const FCameraPresetSettings& NewSettings); // 런타임 중 카메라 설정 동적 변경 시 사용

protected:
    virtual void BeginPlay() override;

	// 충돌 이벤트
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
       bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyPreviewPostProcessing(); // 카메라 프리뷰에 포스트 프로세싱 적용하는 함수
	
	/* 5. 에셋 관리 - DA 연동, 볼륨 우선순위 */
    UPROPERTY(EditAnywhere, Category="05. 에셋 관리", meta=(DisplayName="연동할 DA 원본"))
    UCameraPresetDataAsset* LinkedDataAsset;
    UFUNCTION(CallInEditor, Category="05. 에셋 관리", meta=(DisplayName="DA 불러오기"))
    void LoadFromDataAsset();
    UFUNCTION(CallInEditor, Category="05. 에셋 관리", meta=(DisplayName="DA 저장하기"))
    void SaveToDataAsset();
    UPROPERTY(EditAnywhere, Category="05. 에셋 관리", meta=(DisplayName="볼륨 우선순위", ToolTip="높을수록 우선 적용"))
    int32 Priority = 0;
	
    /* 카메라 설정 01~05 묶는 거대한 토글 */
    UPROPERTY(EditAnywhere, Category="카메라 설정", meta=(ShowOnlyInnerProperties))
    FCameraPresetSettings LocalSettings;
	
	/* 4. 볼륨 레이아웃 */
	UPROPERTY(EditAnywhere, Category="카메라 설정|04. 볼륨 레이아웃", meta=(DisplayName="볼륨 크기 (Half Extent)")) 
	FVector VolumeExtent = FVector(500.f, 500.f, 200.f);
	
	/* 내부 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    UBoxComponent* CollisionBox;
    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    USpringArmComponent* PreviewSpringArm;
    UPROPERTY(VisibleAnywhere, Category="Internal Components")
    UCameraComponent* PreviewCamera;
};