#include "Input/ProjectHInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"


UProjectHInputComponent::UProjectHInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProjectHInputComponent::ApplyInputState(APlayerController* PC, EInputState NewState)
{
	if (!PC || !PC->GetLocalPlayer()) return;

	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	// 기존 매핑 초기화 및 글로벌 매핑 추가
	Subsystem->ClearAllMappings();
	if (IMC_Global) Subsystem->AddMappingContext(IMC_Global, 0);
	
	switch (NewState) // 상태에 따른 IMC 및 마우스/입력 모드 설정
	{
	case EInputState::WorldMap:
		if (IMC_WorldMap) Subsystem->AddMappingContext(IMC_WorldMap, 1);
		PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false).SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock));
		PC->bShowMouseCursor = true;
		break;

	case EInputState::Detailed:
		if (IMC_Detailed) Subsystem->AddMappingContext(IMC_Detailed, 1);
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		break;

	case EInputState::UIOverlay:
		if (IMC_UIOverlay) Subsystem->AddMappingContext(IMC_UIOverlay, 1);
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		break;

	case EInputState::Dialogue: // 대화 넘기기 키(Global)만 활성화, 캐릭터 이동 불가
		PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(true));
		PC->bShowMouseCursor = false;
		break;

	case EInputState::Cinematic: // 연출 시 활성화, 모든 조작 불가
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		break;
	}
}