#include "PlayerMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"

#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UPlayerMovementComponent::UPlayerMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxWalkSpeed = 300.f;
	MaxSprintSpeed = 1000.f;
	WalkSpeedAccel = 1000.f;
	SprintSpeedAccel = 1000.f;
	WalkDeceleration = 0.5f;
	JumpForce = 600.f;
	AirControl = 300.f;
	AirMaxSpeed = 600.f;

	isForwardMoveActive = false;
	isRightMoveActive = false;

	CapsuleRef = nullptr;
	CameraBaseRef = nullptr;
	SkelMeshRef = nullptr;

	CameraVerticalMin = -60.f;
	CameraVerticalMax = 80.f;

	CameraVerticalSpeed = 1.f;
	CameraHorizontalSpeed = 1.f;

	MotionState = EMotionState::Grounded;
}


// Called when the game starts
void UPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AirControl *= 10000.f;
}


// Called every frame
void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalculateMovingForce();
}


void UPlayerMovementComponent::SetComponents(UCapsuleComponent* Capsule, USceneComponent* CameraBase, USkeletalMeshComponent* SkeletalMesh)
{
	CapsuleRef = Capsule;
	if (CapsuleRef != nullptr) {
		CapsuleRef->SetSimulatePhysics(true);
		CapsuleRef->BodyInstance.bLockRotation = true; //Lock all rotations
		CapsuleRef->SetMassOverrideInKg(NAME_None, 50.f, true);
	}

	CameraBaseRef = CameraBase;
	SkelMeshRef = SkeletalMesh;

}



void UPlayerMovementComponent::CalculateMovingForce()
{
	FVector TotalForce = (ForwardMovingForce + RightMovingForce).GetSafeNormal() * GetWorld()->GetDeltaSeconds();
	FVector PlayerVel = CapsuleRef->GetPhysicsLinearVelocity();
	float speedXY = FVector(PlayerVel.X, PlayerVel.Y, 0.f).Size();

	switch (MotionState) {
	case EMotionState::Grounded:
		//If not moving
		if (!isForwardMoveActive && !isRightMoveActive) {

			if (PlayerVel.Size() > 50.f) {
				TotalForce = PlayerVel * -WalkDeceleration;
				TotalForce.Z = 0.f;
				CapsuleRef->SetPhysicsLinearVelocity(TotalForce, true);
			}
		}
		else {
			if (isSprinting && speedXY > MaxSprintSpeed) return;
			if (!isSprinting && speedXY > MaxWalkSpeed) {
				//Rate of deceleration //Low value = Slipping on ice, High value = Abrupt stutter stop
				PlayerVel *= -0.05f;
				CapsuleRef->SetPhysicsLinearVelocity(PlayerVel, true);
			}

			TotalForce *= (WalkSpeedAccel * !isSprinting) + (SprintSpeedAccel * isSprinting);
			CapsuleRef->SetPhysicsLinearVelocity(TotalForce, true);
		}
		break;

	case EMotionState::Aerial:
		if (speedXY > AirMaxSpeed) return;

		//FVector velNormal = CapsuleRef->GetPhysicsLinearVelocity().GetSafeNormal();
		//velNormal.Z = 0.f;
		//FVector dirNormal = SkelMeshRef->GetRightVector();
		//float angleDiffRatio = FVector::DotProduct(velNormal, dirNormal);
		////Control residual velocity to move in the opposite flying direction
		//if (angleDiffRatio < 0.f) {
		//	CapsuleRef->AddForce(-velNormal * AirControl);
		//}

		TotalForce *= AirControl;
		TotalForce.Z = 0.f;

		CapsuleRef->AddForce(TotalForce);
		break;
	}


}

void UPlayerMovementComponent::MoveForward(float AxisValue)
{
	if (CapsuleRef == nullptr || CameraBaseRef == nullptr) return;

	if (AxisValue != 0.f) {
		ForwardMovingForce = CameraBaseRef->GetForwardVector() * AxisValue;
		isForwardMoveActive = true;
	}
	else {
		ForwardMovingForce = FVector::ZeroVector;
		isForwardMoveActive = false;
	}

}

void UPlayerMovementComponent::MoveRight(float AxisValue)
{
	if (CapsuleRef == nullptr || CameraBaseRef == nullptr) return;

	if (AxisValue != 0.f) {
		RightMovingForce = CameraBaseRef->GetRightVector() * AxisValue;
		isRightMoveActive = true;
	}
	else {
		RightMovingForce = FVector::ZeroVector;
		isRightMoveActive = false;
	}

}

void UPlayerMovementComponent::StartSprinting()
{
	if (!isSprinting) isSprinting = true;


}

void UPlayerMovementComponent::StopSprinting()
{
	if (isSprinting) isSprinting = false;
}


void UPlayerMovementComponent::MoveCameraVertical(float AxisValue)
{
	if (CameraBaseRef == nullptr || AxisValue == 0.f) return;

	FRotator camRotation = CameraBaseRef->GetRelativeRotation();
	camRotation.Pitch -= AxisValue * CameraVerticalSpeed;

	if (camRotation.Pitch > CameraVerticalMin&& camRotation.Pitch < CameraVerticalMax)
	{
		CameraBaseRef->SetRelativeRotation(camRotation);
	}
}

void UPlayerMovementComponent::MoveCameraHorizontal(float AxisValue)
{
	if (CameraBaseRef == nullptr || AxisValue == 0.f) return;

	FRotator camRotation = CameraBaseRef->GetRelativeRotation();
	camRotation.Yaw += AxisValue * CameraHorizontalSpeed;
	CameraBaseRef->SetRelativeRotation(camRotation);
}

void UPlayerMovementComponent::Jump()
{
	CapsuleRef->SetPhysicsLinearVelocity(FVector(0.f, 0.f, JumpForce), true);
}

FHitResult UPlayerMovementComponent::GroundTrace()
{
	FHitResult OutHit;

	if (CapsuleRef) {

		FVector Start = CapsuleRef->GetComponentLocation();
		FVector End = ((CapsuleRef->GetUpVector() * -100.f) + Start);

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredComponent(CapsuleRef);

		//TArray<FHitResult> HitResults;

		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 5);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, CollisionParams))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("The Component Being Hit is: %s"), *OutHit.GetComponent()->GetName()));
		}
	}

	return OutHit;
}

void UPlayerMovementComponent::StateChange(EMotionState NewState) {
	if (MotionState == EMotionState::Aerial && NewState == EMotionState::Grounded) {
		FVector velocity = CapsuleRef->GetPhysicsLinearVelocity();
		velocity.Z = 0.f;
		CapsuleRef->SetPhysicsLinearVelocity(velocity, false);
	}

	MotionState = NewState;
}
