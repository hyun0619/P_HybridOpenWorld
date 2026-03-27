#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HybridOpenWorldCharacter.generated.h"


class UInputAction;
class UNiagaraSystem;
class AProjectHPlayerController;

UCLASS(Blueprintable)
class AHybridOpenWorldCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHybridOpenWorldCharacter();
	virtual void Tick(float DeltaSeconds) override;
	
	// 컨트롤러 빙의 시 입력 셋팅해주는 함수
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	// 캐릭터가 컨트롤러에 Possess될 때 호출되는 함수
	virtual void PossessedBy(AController* NewController) override;
	
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
	
	// ★ [추가] 키보드에서 손을 뗐을 때 방향 고정을 풀어줄 함수
	void ResetMoveLatch(const FInputActionValue& Value);
	
	// 매 프레임 Cast X -> 플레이어 컨트롤러 캐싱해둘 약참조 포인터
	UPROPERTY()
	TWeakObjectPtr<AProjectHPlayerController> CachedPC;
};

