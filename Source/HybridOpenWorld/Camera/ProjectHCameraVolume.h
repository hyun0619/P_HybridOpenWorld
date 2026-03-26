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

    // 매니저가 실시간(로컬) 세팅을 가져가도록 함
    FCameraPresetSettings GetCameraSettings() const { return LocalSettings; }

protected:
    virtual void BeginPlay() override;
    
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ApplyPreviewPostProcessing();

protected:
    // ==========================================
    // 00. Camera Settings (영구 저장 및 동기화 툴)
    // ==========================================

    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="연동할 DA 원본 (영구 저장용)"))
    UCameraPresetDataAsset* LinkedDataAsset; 

    // ★ 이 버튼을 누르면 DA 원본의 수치를 가져옵니다.
    UFUNCTION(CallInEditor, Category = "00. Camera Settings", meta=(DisplayName="1. DA 원본 불러오기"))
    void LoadFromDataAsset(); 

    // ★ 이 버튼을 누르면 뷰포트에서 수정한 수치를 DA 원본에 영구 저장합니다! (초기화 방지)
    UFUNCTION(CallInEditor, Category = "00. Camera Settings", meta=(DisplayName="2. DA 원본에 덮어쓰기 (저장)"))
    void SaveToDataAsset(); 

    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta=(DisplayName="우선순위 (Priority)"))
    int32 Priority = 0;

    // 뷰포트에서 직접 깎으면서 테스트할 로컬 세팅값
    UPROPERTY(EditAnywhere, Category = "00. Camera Settings", meta = (ShowOnlyInnerProperties))
    FCameraPresetSettings LocalSettings;


    // ==========================================
    // 내부 컴포넌트
    // ==========================================
    UPROPERTY(VisibleAnywhere, Category = "Internal Components")
    UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere, Category = "Internal Components")
    USpringArmComponent* PreviewSpringArm;
    
    UPROPERTY(VisibleAnywhere, Category = "Internal Components")
    UCameraComponent* PreviewCamera;
};