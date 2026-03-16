#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "ProjectHPlayerController.generated.h"


class AProjectHCameraActor;
class UInputMappingContext;
class UInputAction;

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
	
private:
	// 입력 핸들러 - 필요한 시점에만 호출
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
	/*추후 추가될 기능들 자리*/
	
	UPROPERTY()
	AProjectHCameraActor* MainCameraActor; // 현재 제어 중인 카메라 참조
};
