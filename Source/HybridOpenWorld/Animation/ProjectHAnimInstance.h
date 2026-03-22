#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ProjectHAnimInstance.generated.h"

/**
 * 애니메이션 인스턴스
 */
UCLASS()
class HYBRIDOPENWORLD_API UProjectHAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// 초기화 & 게임 시작 시 한 번 호출 - 에셋 연결
	virtual void NativeInitializeAnimation() override;
	// 업데이트 & 매 프레임 호출 - 속도 등 데이터 갱신
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CurrentSpeed; // 현재 캐릭터 속도
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling; // 공중에 떠있는지 여부

private:
	/*매 프레임 Cast 하는 비용을 아끼기 위해 클래스들 미리 저장*/
	UPROPERTY()
	class ACharacter* OwningCharacter; // 폰
	UPROPERTY()
	class UCharacterMovementComponent* OwningMovementComponent; // 무브먼트 컴포넌트
};