#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ProjectHGameInstance.generated.h"

/**
 * 에셋 비동기 로딩 및 보존할 데이터를 관리하는 클래스
 */
UCLASS()
class HYBRIDOPENWORLD_API UProjectHGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    // 생성자
    //UProjectHGameInstance();

    // 월드맵 레벨에서 플레이어가 마지막으로 있던 좌표 저장
    // 플레이어가 월드맵 복귀할 때 사용할 회전값(바라보는 방향)
    // 현재 플레이 중인 레벨의 이름 저장하여 상태 체크
};
