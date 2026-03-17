#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "ProjectHPlayerController.generated.h"


class UNiagaraSystem;
class AProjectHCameraActor;
class UInputMappingContext;
class UInputAction;
class ULevelDataAsset;
class ULevelMasterAsset;
/**
 * 플레이어 입력 처리, 레벨별 카메라 결정
 */
UCLASS()
class HYBRIDOPENWORLD_API AProjectHPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AProjectHPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	void SetInputModeByType(bool bIsWorldMap); // 조작 모드 변경 (true: 월드맵/마우스, false: 세부지역/WASD)
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "LevelData")
	ULevelMasterAsset* MasterLevelSettings; // 모든 레벨 데이터가 담긴 마스터 에셋 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelData")
	ULevelDataAsset* CurrentLevelData; // 현재 레벨 데이터 에셋
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Context")
	UInputMappingContext* IMC_Global; // 항상 켜있는 기능 (인벤토리, 지도 등)
	UPROPERTY(EditDefaultsOnly, Category="Input|Context")
	UInputMappingContext* IMC_WorldMap; // 월드맵 전용 - 마우스 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Context")
	UInputMappingContext* IMC_Detailed; // 세부지역 전용 - WASD 이동
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Action")
	UInputAction* IA_Move_KeyBoard; // 키보드 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Action")
	UInputAction* IA_Move_MouseClick; // 마우스 클릭 이동 
	/*추후 추가될 기능들 자리*/
	
	// 템플릿에서 가져온 클릭 효과
	UPROPERTY(EditDefaultsOnly, Category="Input|Effect")
	UNiagaraSystem* FXCursor;
	
private:
	// 입력 핸들러 - 필요한 시점에만 호출
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
	/*추후 추가될 기능들 자리*/
	
	UPROPERTY()
	AProjectHCameraActor* MainCameraActor; // 현재 제어 중인 카메라 참조
	float CurrentTrackingSpeed = 5.0f; // 현재 프리셋의 속도값 저장
	bool bCachedFollowPawn = true; 
};
