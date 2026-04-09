#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HybridOpenWorldCharacter.generated.h"

// 컴파일 속도 최적화를 위한 전방 선언
class UInputAction;
class UNiagaraSystem;
class APHPlayerController;

/*
 * 키보드 이동, 마우스 이동 모두 지원하는 하이브리드 캐릭터
 */
UCLASS(Blueprintable)
class AHybridOpenWorldCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHybridOpenWorldCharacter();
	virtual void Tick(float DeltaSeconds) override;
    
	// 컨트롤러 빙의 시 입력을 세팅해주는 함수
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
	// 캐릭터가 컨트롤러에 빙의될 때 호출되는 함수
	virtual void PossessedBy(AController* NewController) override;
    
protected:
	/* IA */
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "키보드 이동"))
	UInputAction* IA_Move_KeyBoard; 
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "마우스 클릭 이동 "))
	UInputAction* IA_Move_MouseClick; 
	UPROPERTY(EditDefaultsOnly, Category="Input|Effect", meta=(ToolTip = "클릭 효과"))
	UNiagaraSystem* FXCursor; 

private:
	/** 입력 처리 함수 */
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
	void ResetMoveLatch(const FInputActionValue& Value); // 키보드에서 손 뗐을 때 호출 -> 방향 고정 초기화 함수
	
	UPROPERTY()
	TWeakObjectPtr<APHPlayerController> CachedPC; // 매 프레임 Cast하는 비용X -> 플레이어 컨트롤러 캐싱해둘 약참조 포인터
    
	/* 조작감 보정 - 카메라가 실시간으로 회전하더라도 플레이어가 손을 떼지 않는 한 
	 * 처음 입력한 월드 기준 방향으로 계속 이동하게 하여 조작 피로도 줄임 */
	FVector LatchedForward;
	FVector LatchedRight;
	
	bool bIsInputLatched = false; // 현재 방향이 고정되어 있는지 여부

	FVector2D LastMoveVector = FVector2D::ZeroVector; // 입력 변화 감지 버퍼
};