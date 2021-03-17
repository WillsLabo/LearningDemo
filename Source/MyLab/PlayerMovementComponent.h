#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine.h"
#include "PlayerMovementComponent.generated.h"

#define debug(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Black, TEXT(x));}

UENUM(BlueprintType)
enum EMotionState
{
	Grounded	UMETA(DisplayName = "Grounded"),
	Aerial		UMETA(DisplayName = "Aerial"),
};

class UCapsuleComponent;
class USceneComponent;
class USkeletalMeshComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYLAB_API UPlayerMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
		void SetComponents(UCapsuleComponent* Capsule, USceneComponent* CameraBase, USkeletalMeshComponent* SkeletalMesh);

	UFUNCTION(BlueprintCallable)
		void MoveForward(float AxisValue);

	UFUNCTION(BlueprintCallable)
		void MoveRight(float AxisValue);

	UFUNCTION(BlueprintCallable)
		void StartSprinting();

	UFUNCTION(BlueprintCallable)
		void StopSprinting();

	UFUNCTION(BlueprintCallable)
		void MoveCameraVertical(float AxisValue);

	UFUNCTION(BlueprintCallable)
		void MoveCameraHorizontal(float AxisValue);

	UFUNCTION(BlueprintCallable)
		void Jump();

	UFUNCTION(BlueprintCallable)
		void StateChange(EMotionState NewState);

private:
	void CalculateMovingForce();
	FHitResult GroundTrace();
		

///Variables
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Max Walk Speed", meta = (UIMin = "0.1", UIMax = "10000.0"))
		float MaxWalkSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Max Sprint Speed", meta = (UIMin = "0.1", UIMax = "10000.0"))
		float MaxSprintSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Walk Deceleration", meta = (UIMin = "0.1", UIMax = "1"))
		float WalkDeceleration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Jump Force", meta = (UIMin = "0.1", UIMax = "2000"))
		float JumpForce;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Air Control", meta = (UIMin = "0", UIMax = "500"))
		float AirControl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings", DisplayName = "Air Max Speed", meta = (UIMin = "0", UIMax = "1000"))
		float AirMaxSpeed;

	UPROPERTY(BlueprintReadOnly)
		bool isForwardMoveActive;

	UPROPERTY(BlueprintReadOnly)
		bool isRightMoveActive;

	UPROPERTY(BlueprintReadOnly)
		bool isSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Camera Settings", DisplayName = "Camera Vertical Rotation Limit (Min)")
		float CameraVerticalMin;

	UPROPERTY(BlueprintReadOnly, Category = "Camera Settings", DisplayName = "Camera Vertical Rotation Limit (Max)")
		float CameraVerticalMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (UIMin = "0.1", UIMax = "20.0"))
		float CameraVerticalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (UIMin = "0.1", UIMax = "30.0"))
		float CameraHorizontalSpeed;


private:
	UCapsuleComponent* CapsuleRef;
	USceneComponent* CameraBaseRef;
	USkeletalMeshComponent* SkelMeshRef;

	FVector ForwardMovingForce;
	FVector RightMovingForce;

	float WalkSpeedAccel;
	float SprintSpeedAccel;

	EMotionState MotionState;

	FORCEINLINE void debugf(float val) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Value: %f"), val));
		}
	}

	FORCEINLINE void debugf(FVector vec) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Value: %f, %f, %f"), vec.X, vec.Y, vec.Z));
		}
	}
};
