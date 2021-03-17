// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"

UCLASS()
class MYLAB_API ABoid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FVector Cohesion();

	UFUNCTION(BlueprintCallable)
	FVector Separation();

	UFUNCTION(BlueprintCallable)
	FVector Alignment();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SetSpawnPointLocation(FVector NewSpawnLocation);

private:
	//For debugging, drawing line trace on screen
	UFUNCTION()
	FHitResult LineTrace(FVector Start, FVector End);

	UFUNCTION()
	FVector TraceObstacle();

	UFUNCTION()
	FVector MoveTowardOrigin();

	UFUNCTION()
	FVector OrthonormalVelocity(bool bClockwise);

	UFUNCTION()
	void AutoOrient();

	FORCEINLINE void Debug(float Value) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Value: %f"), Value));
		}
	}

	FORCEINLINE void Debug(FVector Vector) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Value: %f, %f, %f"), Vector.X, Vector.Y, Vector.Z));
		}
	}

	FORCEINLINE void Debug(FString String) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, String);
		}
	}

//Variables
public:
	UPROPERTY(BlueprintReadWrite)
	TSet<AActor*> Boids;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USphereComponent* RootSphere;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USphereComponent* SensingSphere;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USceneComponent* MeshParent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float SpeedScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float CohesionRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float SeparationLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float SeparationRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float AlignmentRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float AvoidanceRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float ExpandRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float ReturnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "1.0"))
	float VortexRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	bool VortexClockwise;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float ReOrientRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "10000.0"))
	float TraceLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats", meta = (UIMin = "0.0", UIMax = "10000.0"))
	float DistanceFromSpawn;

private:
	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	FTimerHandle FT_Handle_AutoOrient;
};
