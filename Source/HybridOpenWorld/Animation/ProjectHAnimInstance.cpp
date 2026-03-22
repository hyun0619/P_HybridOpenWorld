#include "Animation/ProjectHAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


void UProjectHAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 이 애니메이션의 주인을 찾아 ACharacter로 캐스팅하여 저장
	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    
	if (IsValid(OwningCharacter))
	{
		// 주인의 움직임을 담당하는 컴포넌트를 캐싱
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UProjectHAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 엔진 로딩 순서상 초기화 때 폰을 못 가져올 때 다시 시도 - 방어 코드
	if (!IsValid(OwningCharacter))
	{
		OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (IsValid(OwningCharacter))
		{
			OwningMovementComponent = OwningCharacter->GetCharacterMovement();
		}
	}

	// 캐릭터와 컴포넌트가 모두 안전하게 존재할 때만 연산
	if (IsValid(OwningCharacter) && IsValid(OwningMovementComponent))
	{
		// 속도 계산 최적화
		CurrentSpeed = OwningCharacter->GetVelocity().Size2D();

		// 공중 체공 여부
		bIsFalling = OwningMovementComponent->IsFalling();
	}
}