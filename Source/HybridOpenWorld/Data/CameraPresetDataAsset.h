#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraPresetDataAsset.generated.h"

// ──────────────────────────────────────────────
// Enums
// ──────────────────────────────────────────────

/** 카메라 볼륨의 동작 유형 */
UENUM(BlueprintType)
enum class ECameraVolumeType : uint8
{
	/** 추적 카메라: 플레이어를 따라가되, 볼륨 범위 내에서 이동 */
	Dynamic		UMETA(DisplayName = "Dynamic (추적)"),
	/** 고정 카메라: 볼륨에 설정된 월드 위치/회전 고정 */
	Static		UMETA(DisplayName = "Static (고정)")
};

/** 카메라 투영 모드 */
UENUM(BlueprintType)
enum class ECameraProjectionType : uint8
{
	Perspective		UMETA(DisplayName = "Perspective (원근)"),
	Orthographic	UMETA(DisplayName = "Orthographic (직교)")
};

/** 볼륨 경계에서 카메라 이동을 차단할 면 (비트 플래그) */
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


// ──────────────────────────────────────────────
// ★ 카메라 설정 구조체 (기존 필드 전부 보존 + 신규 필드 추가)
// ──────────────────────────────────────────────

USTRUCT(BlueprintType)
struct HYBRIDOPENWORLD_API FCameraPresetSettings
{
	GENERATED_BODY()

	// ══════════════════════════════════════════
	// 01. 볼륨 동작 모드
	// ══════════════════════════════════════════

	/** 볼륨 유형: Dynamic=플레이어 추적, Static=고정 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01. Volume Mode", meta=(DisplayName="볼륨 유형"))
	ECameraVolumeType VolumeType = ECameraVolumeType::Dynamic;

	// ══════════════════════════════════════════
	// 02. 카메라 렌즈 및 위치 (기존 필드 유지)
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="카메라 암 길이"))
	float TargetArmLength = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="시야각 (FOV)", EditCondition="ProjectionType == ECameraProjectionType::Perspective", ClampMin="5.0", ClampMax="170.0"))
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="고정 회전값"))
	FRotator Rotation = FRotator(-30.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="카메라 중심점 오프셋 (SocketOffset)"))
	FVector CameraOffset = FVector::ZeroVector;

	/** 카메라 롤 (도) — Rotation.Roll과 독립적으로 추가 롤 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="추가 카메라 롤", ClampMin="-180.0", ClampMax="180.0"))
	float CameraRoll = 0.0f;

	/** 포커스 포인트 오프셋 — 캐릭터 기준으로 카메라가 바라볼 지점 조정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02. Camera Transform", meta=(DisplayName="포커스 포인트 오프셋"))
	FVector FocusPointOffset = FVector::ZeroVector;

	// ══════════════════════════════════════════
	// 03. Static 카메라 전용 (VolumeType == Static 일 때)
	// ══════════════════════════════════════════

	/** Static 모드에서 카메라가 위치할 월드 오프셋 (볼륨 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="03. Static Camera", meta=(DisplayName="고정 카메라 위치 오프셋", EditCondition="VolumeType == ECameraVolumeType::Static"))
	FVector StaticCameraOffset = FVector(0.f, 0.f, 800.f);

	/** Static 모드에서 카메라 회전 (볼륨 기준 로컬 회전) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="03. Static Camera", meta=(DisplayName="고정 카메라 회전", EditCondition="VolumeType == ECameraVolumeType::Static"))
	FRotator StaticCameraRotation = FRotator(-60.f, 0.f, 0.f);

	// ══════════════════════════════════════════
	// 04. 투영 모드
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="04. Projection", meta=(DisplayName="투영 모드"))
	ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;

	/** 직교(Orthographic) 모드 시 너비 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="04. Projection", meta=(DisplayName="직교 너비 (OrthoWidth)", EditCondition="ProjectionType == ECameraProjectionType::Orthographic", ClampMin="100.0"))
	float OrthoWidth = 2048.0f;

	// ══════════════════════════════════════════
	// 05. 이동 및 추적 (기존 필드 유지 + 래그 확장)
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="캐릭터 추적 여부"))
	bool bFollowPawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="캐릭터를 따라가는 속도", EditCondition="bFollowPawn"))
	float TrackingInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="카메라 전환 시간 (초)"))
	float BlendTime = 2.0f;

	/** 위치 래그 활성화 (SpringArm CameraLag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="위치 래그 활성화"))
	bool bEnableLocationLag = true;

	/** 위치 래그 속도 (낮을수록 부드럽게 따라옴) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="위치 래그 속도", EditCondition="bEnableLocationLag", ClampMin="0.5", ClampMax="50.0"))
	float LocationLagSpeed = 3.0f;

	/** 회전 래그 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="회전 래그 활성화"))
	bool bEnableRotationLag = true;

	/** 회전 래그 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="05. Tracking", meta=(DisplayName="회전 래그 속도", EditCondition="bEnableRotationLag", ClampMin="0.5", ClampMax="50.0"))
	float RotationLagSpeed = 10.0f;

	// ══════════════════════════════════════════
	// 06. 바운드 블로킹 (카메라 이동 제한)
	// ══════════════════════════════════════════

	/** 볼륨 경계에서 카메라 이동 제한 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="06. Bounds Blocking", meta=(DisplayName="바운드 블로킹 활성화"))
	bool bEnableBoundsBlocking = false;

	/** 차단할 면 방향 (비트 플래그) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="06. Bounds Blocking", meta=(DisplayName="차단 면 방향", Bitmask, BitmaskEnum="/Script/HybridOpenWorld.ECameraBoundsBlockFlags", EditCondition="bEnableBoundsBlocking"))
	int32 BoundsBlockFlags = static_cast<int32>(ECameraBoundsBlockFlags::BlockAllXY);

	/** 바운드 안쪽 패딩 (카메라가 경계 직전에 멈추도록) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="06. Bounds Blocking", meta=(DisplayName="바운드 안쪽 패딩", EditCondition="bEnableBoundsBlocking", ClampMin="0.0"))
	float BoundsPadding = 50.0f;

	// ══════════════════════════════════════════
	// 07. 카메라 충돌
	// ══════════════════════════════════════════

	/** 카메라 충돌 감지 활성화 (SpringArm DoCollisionTest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="07. Collision", meta=(DisplayName="카메라 충돌 활성화"))
	bool bEnableCameraCollision = false;

	/** 충돌 프로브 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="07. Collision", meta=(DisplayName="충돌 프로브 반경", EditCondition="bEnableCameraCollision", ClampMin="1.0"))
	float CollisionProbeRadius = 12.0f;

	// ══════════════════════════════════════════
	// 08. Pawn Control Rotation
	// ══════════════════════════════════════════

	/** 폰 컨트롤 로테이션을 카메라에 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="08. Pawn Control", meta=(DisplayName="폰 컨트롤 로테이션 사용"))
	bool bUsePawnControlRotation = false;

	// ══════════════════════════════════════════
	// 09. 포스트 프로세스 (기존 필드 그대로 유지)
	// ══════════════════════════════════════════

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(DisplayName="틸트 쉬프트 활성화"))
	bool bEnableTiltShift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="조리개 (F-Stop)"))
	float ApertureFStop = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="센서 폭 (mm)"))
	float SensorWidth = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="수동 초점 거리"))
	float ManualFocusDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 가까운 블러 세기"))
	float NearBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="카메라와 먼 블러 세기"))
	float FarBlurRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="09. Post Process", meta=(EditCondition="bEnableTiltShift", DisplayName="블러 경계 부드러움 세기"))
	float FarTransitionRegion = 1000.f;
};


// ──────────────────────────────────────────────
// ★ 데이터 에셋 클래스 (기존 구조 유지)
// ──────────────────────────────────────────────

UCLASS(BlueprintType)
class HYBRIDOPENWORLD_API UCameraPresetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Camera Settings", meta=(ShowOnlyInnerProperties))
	FCameraPresetSettings Settings;
};