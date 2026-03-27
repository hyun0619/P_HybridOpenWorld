#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

// ──────────────────────────────────────────────
// Enums
// ──────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECameraVolumeType : uint8
{
	Dynamic		UMETA(DisplayName = "Dynamic (추적)"),
	Static		UMETA(DisplayName = "Static (고정)")
};

UENUM(BlueprintType)
enum class ECameraProjectionType : uint8
{
	Perspective		UMETA(DisplayName = "Perspective (원근)"),
	Orthographic	UMETA(DisplayName = "Orthographic (직교)")
};

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsBitmaskValues = "true"))
enum class ECameraBoundsBlockFlags : uint8
{
	None		= 0			UMETA(Hidden),
	BlockMinX	= 1 << 0	UMETA(DisplayName = "-X 차단"),
	BlockMaxX	= 1 << 1	UMETA(DisplayName = "+X 차단"),
	BlockMinY	= 1 << 2	UMETA(DisplayName = "-Y 차단"),
	BlockMaxY	= 1 << 3	UMETA(DisplayName = "+Y 차단"),
	BlockMinZ	= 1 << 4	UMETA(DisplayName = "-Z 차단"),
	BlockMaxZ	= 1 << 5	UMETA(DisplayName = "+Z 차단"),
	BlockAllXY	= BlockMinX | BlockMaxX | BlockMinY | BlockMaxY	UMETA(DisplayName = "XY 전체 차단"),
	BlockAll	= 0x3F		UMETA(DisplayName = "전축 차단")
};
ENUM_CLASS_FLAGS(ECameraBoundsBlockFlags);


// ══════════════════════════════════════════════════════════════
// FCameraPresetSettings
//
// [변경 이력]
//   - CameraRoll 제거 → Rotation.Roll 로 통합
//   - FieldOfView를 Dynamic/Static 모드별로 분리 (StaticFieldOfView)
//   - OrthoWidth를 Dynamic/Static 모드별로 분리 (StaticOrthoWidth)
//   - ExitBlendTime 추가 (볼륨 퇴장 시 전환 속도 독립 제어)
//   - 카테고리를 A/B/C 3개로 통합 (디테일창 상단에 표시)
//   - 모든 필드에 ToolTip 추가
// ══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
	GENERATED_BODY()

	// ══════════════════════════════════════════
	// [A] 카메라 기본 설정
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본",
		meta=(DisplayName="볼륨 유형",
			  ToolTip="Dynamic=플레이어를 따라감, Static=볼륨 기준 고정 위치"))
	ECameraVolumeType VolumeType = ECameraVolumeType::Dynamic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본",
		meta=(DisplayName="투영 모드",
			  ToolTip="Perspective=일반 3D, Orthographic=등각투영(쿼터뷰/2D)"))
	ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본",
		meta=(DisplayName="캐릭터 추적",
			  ToolTip="켜면 카메라가 플레이어 위치를 따라갑니다"))
	bool bFollowPawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본",
		meta=(DisplayName="폰 컨트롤 로테이션",
			  ToolTip="켜면 마우스 입력에 의한 Pawn 컨트롤 로테이션이 카메라 회전에 적용됩니다"))
	bool bUsePawnControlRotation = false;

	// ─── Dynamic 모드 전용 ────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 암 길이",
			  ToolTip="SpringArm 길이. 숫자가 클수록 카메라가 멀어집니다 (일반 500~2000)",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic", ClampMin="0.0"))
	float TargetArmLength = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 회전값",
			  ToolTip="SpringArm 회전. Pitch=-30이면 30도 위에서 내려다봅니다. Roll로 카메라 기울기 조절 가능",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic"))
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="카메라 중심점 오프셋",
			  ToolTip="SocketOffset. X=전후 Y=좌우 Z=상하. 캐릭터 머리 위를 비추려면 Z를 올립니다",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic"))
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="포커스 포인트 오프셋",
			  ToolTip="카메라가 따라가는 지점을 캐릭터 위치에서 이동. Z=100이면 머리 근처를 추적",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic"))
	FVector FocusPointOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="시야각 (FOV)",
			  ToolTip="Perspective 모드 전용. 90이 기본. 낮으면 줌인 높으면 어안렌즈",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic && ProjectionType == ECameraProjectionType::Perspective",
			  ClampMin="5.0", ClampMax="170.0"))
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Dynamic",
		meta=(DisplayName="직교 너비 (OrthoWidth)",
			  ToolTip="Orthographic 모드 전용. 클수록 넓은 영역이 보임 (일반 1024~4096)",
			  EditCondition="VolumeType == ECameraVolumeType::Dynamic && ProjectionType == ECameraProjectionType::Orthographic",
			  ClampMin="100.0"))
	float OrthoWidth = 2048.0f;

	// ─── Static 모드 전용 ─────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Static",
		meta=(DisplayName="고정 카메라 위치 오프셋",
			  ToolTip="볼륨 중심으로부터의 카메라 위치. Z=800이면 800유닛 위에 카메라가 놓입니다",
			  EditCondition="VolumeType == ECameraVolumeType::Static"))
	FVector StaticCameraOffset = FVector(0.f, 0.f, 800.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Static",
		meta=(DisplayName="고정 카메라 회전",
			  ToolTip="볼륨 회전 기준. Pitch=-90이면 완전 탑뷰",
			  EditCondition="VolumeType == ECameraVolumeType::Static"))
	FRotator StaticCameraRotation = FRotator(-60.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Static",
		meta=(DisplayName="시야각 (FOV)",
			  ToolTip="Static 카메라 FOV. Perspective 모드에서만 유효",
			  EditCondition="VolumeType == ECameraVolumeType::Static && ProjectionType == ECameraProjectionType::Perspective",
			  ClampMin="5.0", ClampMax="170.0"))
	float StaticFieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="A. 카메라 기본|Static",
		meta=(DisplayName="직교 너비 (OrthoWidth)",
			  ToolTip="Static 카메라 OrthoWidth. Orthographic 모드에서만 유효",
			  EditCondition="VolumeType == ECameraVolumeType::Static && ProjectionType == ECameraProjectionType::Orthographic",
			  ClampMin="100.0"))
	float StaticOrthoWidth = 2048.0f;

	// ══════════════════════════════════════════
	// [B] 전환 및 래그
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="진입 전환 시간 (초)",
			  ToolTip="이 볼륨에 들어올 때의 블렌딩 시간. 0=즉시 전환(하드컷)",
			  ClampMin="0.0"))
	float BlendTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="퇴장 전환 시간 (초)",
			  ToolTip="이 볼륨에서 나갈 때의 블렌딩 시간. -1이면 복귀 대상의 진입 전환 시간을 따름. 0=즉시 전환",
			  ClampMin="-1.0"))
	float ExitBlendTime = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="추적 보간 속도",
			  ToolTip="캐릭터를 따라가는 속도 (일반 3~15). bFollowPawn 활성 시에만 유효",
			  EditCondition="bFollowPawn", ClampMin="0.5", ClampMax="50.0"))
	float TrackingInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="위치 래그 활성화",
			  ToolTip="켜면 카메라가 목표 위치에 부드럽게 도달합니다"))
	bool bEnableLocationLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="위치 래그 속도",
			  ToolTip="낮을수록 부드럽고 느리게 (일반 3~10)",
			  EditCondition="bEnableLocationLag", ClampMin="0.5", ClampMax="50.0"))
	float LocationLagSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="회전 래그 활성화",
			  ToolTip="켜면 카메라 회전이 부드럽게 보간됩니다"))
	bool bEnableRotationLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="B. 전환 및 래그",
		meta=(DisplayName="회전 래그 속도",
			  ToolTip="낮을수록 부드러운 회전 (일반 5~15)",
			  EditCondition="bEnableRotationLag", ClampMin="0.5", ClampMax="50.0"))
	float RotationLagSpeed = 10.0f;

	// ══════════════════════════════════════════
	// [C] 바운드 / 충돌 / 포스트프로세스
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|바운드 블로킹",
		meta=(DisplayName="바운드 블로킹 활성화",
			  ToolTip="카메라가 볼륨 경계 밖으로 나가지 못하게 합니다"))
	bool bEnableBoundsBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|바운드 블로킹",
		meta=(DisplayName="차단 면 방향",
			  Bitmask, BitmaskEnum="/Script/HybridOpenWorld.ECameraBoundsBlockFlags",
			  EditCondition="bEnableBoundsBlocking"))
	int32 BoundsBlockFlags = static_cast<int32>(ECameraBoundsBlockFlags::BlockAllXY);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|바운드 블로킹",
		meta=(DisplayName="바운드 안쪽 패딩",
			  EditCondition="bEnableBoundsBlocking", ClampMin="0.0"))
	float BoundsPadding = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|카메라 충돌",
		meta=(DisplayName="카메라 충돌 활성화",
			  ToolTip="벽이 카메라를 가릴 때 카메라를 앞으로 당깁니다"))
	bool bEnableCameraCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|카메라 충돌",
		meta=(DisplayName="충돌 프로브 반경",
			  EditCondition="bEnableCameraCollision", ClampMin="1.0"))
	float CollisionProbeRadius = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(DisplayName="틸트 쉬프트 활성화",
			  ToolTip="화면 상하단을 흐리게 만들어 미니어처 효과를 줍니다"))
	bool bEnableTiltShift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="조리개 (F-Stop)",
			  ToolTip="낮을수록 배경 흐림이 강함 (일반 0.5~4.0)"))
	float ApertureFStop = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="센서 폭 (mm)"))
	float SensorWidth = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="수동 초점 거리"))
	float ManualFocusDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="근거리 블러 세기"))
	float NearBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="원거리 블러 세기"))
	float FarBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C. 고급 설정|포스트 프로세스",
		meta=(EditCondition="bEnableTiltShift", DisplayName="블러 경계 부드러움"))
	float FarTransitionRegion = 1000.f;

	// ─── 헬퍼 ─────────────────────────────────
	float GetEffectiveFOV() const
	{
		return (VolumeType == ECameraVolumeType::Static) ? StaticFieldOfView : FieldOfView;
	}
	float GetEffectiveOrthoWidth() const
	{
		return (VolumeType == ECameraVolumeType::Static) ? StaticOrthoWidth : OrthoWidth;
	}
};


UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Camera Settings", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings Settings;
};