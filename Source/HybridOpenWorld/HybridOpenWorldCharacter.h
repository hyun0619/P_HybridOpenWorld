#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HybridOpenWorldCharacter.generated.h"


class UInputAction;
class UNiagaraSystem;

UCLASS(Blueprintable)
class AHybridOpenWorldCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHybridOpenWorldCharacter();
	virtual void Tick(float DeltaSeconds) override;
	
	// 컨트롤러 빙의 시 입력 셋팅해주는 함수
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "키보드 이동"))
	UInputAction* IA_Move_KeyBoard; // 키보드 이동
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "마우스 클릭 이동 "))
	UInputAction* IA_Move_MouseClick; // 마우스 클릭 이동 

	UPROPERTY(EditDefaultsOnly, Category="Input|Effect", meta=(ToolTip = "클릭 효과"))
	UNiagaraSystem* FXCursor; // 템플릿에서 가져온 클릭 효과

private:
	// 입력 핸들러 - 필요한 시점에만 호출
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
};

