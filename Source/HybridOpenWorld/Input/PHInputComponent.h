#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PHInputComponent.generated.h"


class UInputMappingContext;

/**
 * 게임 내 발생할 수 있는 모든 조작 상태
 */
UENUM(BlueprintType)
enum class EInputState : uint8 
{ 
	WorldMap, // 월드맵 (마우스 이동)
	Detailed, // 세부지역 (WASD 이동)
	UIOverlay, // UI 상호작용 (ex.인벤토리/전체맵)
	Dialogue, // 대화 중 (이동 불가)
	Cinematic // 시네마틱 (모든 입력 차단)
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYBRIDOPENWORLD_API UPHInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPHInputComponent();
	void ApplyInputState(APlayerController* PC, EInputState NewState); // 컨트롤러가 상태 바꿀 때 호출하는 함수

protected:
	/* IMC 설정 */
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "항상 켜있는 기능 (인벤토리, 지도 등)"))
	UInputMappingContext* IMC_Global; // 항상 켜있는 기능 (인벤토리, 지도 등)
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "월드맵 전용 - 마우스 이동"))
	UInputMappingContext* IMC_WorldMap; // 월드맵 전용 - 마우스 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "세부지역 전용 - WASD 이동"))
	UInputMappingContext* IMC_Detailed; // 세부지역 전용 - WASD 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Context", meta=(ToolTip = "UI 상호작용 - 이동X, 마우스 조작"))
	UInputMappingContext* IMC_UIOverlay; // UI 전용 조작
};
