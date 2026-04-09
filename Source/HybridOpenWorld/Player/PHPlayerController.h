#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Data/LevelSettingsData.h"
#include "PHPlayerController.generated.h"


class APHCameraActor;
class UInputAction;
class UInputMappingContext;
class UGameMasterAsset;
class UPHInputComponent;
class UPHLookAroundComponent;

/*
 * 임력 상태 관리, 카메라 액터 연결, 레벨 초기화 셋팅 담당하는 컨트롤러
 */
UCLASS()
class HYBRIDOPENWORLD_API APHPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APHPlayerController();
    
    APHCameraActor* GetMainCameraActor() const; // 현재 제어 중인 메인 카메라 액터 반환
    
    // 입력 상태 변경
    UFUNCTION(BlueprintCallable, Category="InputState")
    void ChangeInputState(EInputState NewState);
    UFUNCTION(BlueprintCallable, Category="InputState")
    void RevertToDefaultState();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    
    /** 입력 액션 및 컴포넌트 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
    UInputAction* IA_LookAround; // 둘러보기 액션
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UPHInputComponent* InputManager;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UPHLookAroundComponent* EdgeScrollComponent;
    
    /** 상태 관리 변수 */
    EInputState DefaultState;
    EInputState CurrentState;

    /** 레벨 데이터 시스템 */
    UPROPERTY(EditDefaultsOnly, Category="LevelData")
    UGameMasterAsset* MasterLevelSettings;
    
    FLevelSettingsRow CurrentLevelRow; // 현재 맵의 데이터 테이블 정보 캐싱

private:
    /** 입력 핸들러 */
    void OnLookAroundStarted(const FInputActionValue& Value);
    void OnLookAroundCompleted(const FInputActionValue& Value);
    
    /** 초기화 내부 함수 */
    void InitEssentialReferences();
    void FetchLevelData();
    void ApplyInitialLevelSetup();
    void HandleInitialSpawn();

    UPROPERTY()
    APHCameraActor* MainCameraActor; // 월드에 배치된 메인 카메라 액터
 
    bool bHasValidLevelData = false; // 레벨 데이터를 성공적으로 읽었는지 여부
};