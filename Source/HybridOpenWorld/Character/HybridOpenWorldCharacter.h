#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HybridOpenWorldCharacter.generated.h"

// 컴파일 속도 최적화를 위한 전방 선언
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
    
	// 컨트롤러 빙의 시 입력을 세팅해주는 함수 (Enhanced Input 바인딩)
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
	// 캐릭터가 컨트롤러에 빙의(Possess)될 때 호출되는 함수
	virtual void PossessedBy(AController* NewController) override;
    
protected:
	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "키보드 이동"))
	UInputAction* IA_Move_KeyBoard; 

	UPROPERTY(EditDefaultsOnly, Category="Input|Action", meta=(ToolTip = "마우스 클릭 이동 "))
	UInputAction* IA_Move_MouseClick; 

	UPROPERTY(EditDefaultsOnly, Category="Input|Effect", meta=(ToolTip = "클릭 효과"))
	UNiagaraSystem* FXCursor; 

private:
	// ─── 입력 핸들러 ───
	void HandleMove_KeyBoard(const FInputActionValue& Value);
	void HandleMove_MouseClick();
    
	// 키보드에서 완전히 손을 뗐을 때 호출하여 방향 고정을 초기화하는 함수
	void ResetMoveLatch(const FInputActionValue& Value);
    
	// ─── 최적화 변수 ───
	// 매 프레임 Cast하는 비용을 없애기 위해 플레이어 컨트롤러를 캐싱해둘 약참조 포인터
	UPROPERTY()
	TWeakObjectPtr<AProjectHPlayerController> CachedPC;
    
	// ─── 조작감 보정(Smart Latch) 변수 ───
	// 카메라가 회전해도 캐릭터가 기존 방향을 유지하도록 고정해둘 벡터
	FVector LatchedForward;
	FVector LatchedRight;
    
	// 현재 방향이 고정되어 있는지 여부
	bool bIsInputLatched = false;

	// 이전 프레임의 입력값 (새로운 키 입력이나 떼기를 감지하기 위함)
	FVector2D LastMoveVector = FVector2D::ZeroVector;
};