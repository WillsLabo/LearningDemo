// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.10f;	//Manually set a slower tick rate

	RootSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RootSphere"));
	SensingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	MeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));

	SetRootComponent(RootSphere);
	SensingSphere->SetupAttachment(RootSphere);
	MeshParent->SetupAttachment(RootSphere);
	MeshComp->SetupAttachment(MeshParent);

	if (RootSphere != nullptr) {
		RootSphere->SetSimulatePhysics(true);
		RootSphere->SetEnableGravity(false);
		RootSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		RootSphere->BodyInstance.bLockRotation = true;
		RootSphere->SetMassOverrideInKg(NAME_None, 1.f, true);
	}
	if (SensingSphere != nullptr) {
		SensingSphere->SetSphereRadius(250.f);
		SensingSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SensingSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	}
	if (MeshComp != nullptr) {
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SensingSphere->OnComponentBeginOverlap.AddDynamic(this, &ABoid::OnBeginOverlap);
	SensingSphere->OnComponentEndOverlap.AddDynamic(this, &ABoid::OnEndOverlap);

	//Adding a tag just in case I need to filter overlapped actors
	Tags.Add(FName("Boid"));

	//Initialize variables at construction
	SpeedScale = 10000.f;
	CohesionRate = 0.25f;
	SeparationLength = 200.f;
	SeparationRate = 0.4f;
	AlignmentRate = 0.2f;
	AvoidanceRate = 0.0f;
	ExpandRate = 0.1f;
	ReturnRate = 0.001f;
	VortexRate = 0.f;
	VortexClockwise = true;
	ReOrientRate = 0.1f;

	TraceLength = 400.f;
	DistanceFromSpawn = 1000.f;

}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();

	//Calculate a constant for function timer Tick Rate
	float OrientUpdateRate = 1.f / 30.f; //30 fps

	//Set Timer
	GetWorld()->GetTimerManager().SetTimer(FT_Handle_AutoOrient, this, &ABoid::AutoOrient, OrientUpdateRate, true);

	//Get a list in the beginning
	GetOverlappingActors(Boids, TSubclassOf<ABoid>());
}

// Called every frame
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector TotalVelocity;
	TotalVelocity = MoveTowardOrigin() + OrthonormalVelocity(VortexClockwise);// +TraceObstacle();
	TotalVelocity = TotalVelocity.GetSafeNormal();

	TotalVelocity += Cohesion() + Separation() + Alignment();

	TotalVelocity = TotalVelocity.GetSafeNormal() * SpeedScale;

	//TotalVelocity actually alter the change in acceleration
	RootSphere->AddForce(TotalVelocity, NAME_None, true);
}


FVector ABoid::Cohesion()
{
	FVector DirectionToClusterMid = FVector::ZeroVector;
	FVector AveragePosition = FVector::ZeroVector;

	if (Boids.Num() > 0) {
		for (AActor* actor : Boids) {
			AveragePosition += actor->GetActorLocation();
		}
		AveragePosition /= Boids.Num();
		DirectionToClusterMid = (AveragePosition - RootSphere->GetRelativeLocation());
	}

	return DirectionToClusterMid.GetSafeNormal() * CohesionRate;
}

FVector ABoid::Separation()
{
	FVector DirectionAwayFromCrowd = FVector::ZeroVector;

	if (Boids.Num() > 0) {
		for (AActor* actor : Boids) {
			FVector directionFromOther = RootSphere->GetComponentLocation() - actor->GetActorLocation();
			float distanceToOther = directionFromOther.Size();

			if (distanceToOther < SeparationLength) {
				if (distanceToOther == 0.f) distanceToOther = 0.000001f;
				DirectionAwayFromCrowd += directionFromOther.GetSafeNormal() * (SeparationRate / distanceToOther);
			}
		}
		DirectionAwayFromCrowd /= Boids.Num();
	}

	return DirectionAwayFromCrowd.GetSafeNormal() * SeparationRate;
}

FVector ABoid::Alignment()
{
	FVector AverageClusterVelocity = FVector::ZeroVector;

	if (Boids.Num() > 0) {
		for (AActor* actor : Boids) {
			AverageClusterVelocity += actor->GetVelocity().GetSafeNormal();
		}
		AverageClusterVelocity /= Boids.Num();

		//Steering formula, that i'm not using...
		//AverageClusterVelocity = AverageClusterVelocity - RootSphere->GetComponentVelocity();
	}

	return AverageClusterVelocity.GetSafeNormal() * AlignmentRate;
}

void ABoid::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Update the List if something new enters the area
	GetOverlappingActors(Boids, TSubclassOf<ABoid>());
}

void ABoid::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//Update the list if something leaves the area
	GetOverlappingActors(Boids, TSubclassOf<ABoid>());
}

void ABoid::SetSpawnPointLocation(FVector NewSpawnLocation)
{
	SpawnLocation = NewSpawnLocation;

	//Move away from spawn point
	FVector NewVelocity;
	NewVelocity = (RootSphere->GetRelativeLocation() - SpawnLocation).GetSafeNormal() * SpeedScale * ExpandRate;

	RootSphere->AddForce(NewVelocity, NAME_None, true);
}

FHitResult ABoid::LineTrace(FVector Start, FVector End)
{
	FHitResult OutHit;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredComponent(SensingSphere);
	CollisionParams.AddIgnoredComponent(RootSphere);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 5);

	//if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, CollisionParams))
	//{
	//	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("The Component Being Hit is: %s"), *OutHit.GetComponent()->GetName()));
	//}

	return OutHit;
}

FVector ABoid::TraceObstacle()
{
	//Calculations to move away from obstacles in the way
	//Unusable for now...

	FHitResult OutHit;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredComponent(SensingSphere);
	CollisionParams.AddIgnoredComponent(RootSphere);

	/*for (AActor* actor : Boids) {
		CollisionParams.AddIgnoredActor(actor);
	}*/

	//TArray<FHitResult> HitResults;
	FVector AvoidVelocity;
	FVector Start = RootSphere->GetRelativeLocation();
	FVector End = Start + (MeshParent->GetForwardVector() * TraceLength);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, 1, 0, 5);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, CollisionParams))
	{
		//FVector SurfaceNormal = OutHit.ImpactPoint + (OutHit.ImpactNormal * 150.f);
		FVector ImpactDirection = Start - OutHit.ImpactPoint;
		FVector AvoidDirection = FVector::CrossProduct(ImpactDirection, FVector(0.f, 0.f, 1.f));
		float DistanceRatio = TraceLength / OutHit.Distance;

		return AvoidDirection * DistanceRatio * AvoidanceRate;

		//x=xcos - ysin
		//y=xsin + ycos
		//Cosine = Adj/Hyp; Hyp = Adj/Cos(Angle);
		/*float Angle = FMath::DegreesToRadians(60.f);
		float X = (Forward.X * FMath::Cos(Angle)) - (Forward.Z * FMath::Sin(Angle));
		float Y = (Forward.X * FMath::Sin(Angle)) + (Forward.Z * FMath::Cos(Angle));*/
	}
	return FVector::ZeroVector;
}

FVector ABoid::MoveTowardOrigin()
{
	//Anchors the boid back to spawn point if it moves too far away

	FVector DirectionToSpawn = SpawnLocation - RootSphere->GetRelativeLocation();
	float DistanceAway = DirectionToSpawn.SizeSquared();

	if (DistanceAway > (DistanceFromSpawn * DistanceFromSpawn)) {
		return DirectionToSpawn * ReturnRate;
	}

	return FVector::ZeroVector;
}

FVector ABoid::OrthonormalVelocity(bool bClockwise)
{
	//Add perpendicular velocity of a vector between boid and spawn point
	//This creates a circular-like motion overtime

	FVector Direction = (SpawnLocation - RootSphere->GetComponentVelocity()).GetSafeNormal();

	if (bClockwise) {
		Direction = FVector(Direction.Y, -Direction.X, Direction.Z);
	}
	else {
		Direction = FVector(-Direction.Y, Direction.X, Direction.Z);
	}

	return Direction.GetSafeNormal() * VortexRate;
}

void ABoid::AutoOrient()
{
	//Orient face to velocity direction

	FRotator TargetOrientation = FRotationMatrix::MakeFromX(RootSphere->GetComponentVelocity().GetSafeNormal()).Rotator();
	FRotator CurrentOrientation = MeshParent->GetForwardVector().Rotation();

	FRotator RotationDiff = TargetOrientation - CurrentOrientation;
	RotationDiff *= ReOrientRate;
	MeshParent->AddRelativeRotation(RotationDiff);

	//2nd optional method for rotation adjustment, this one will snap the rotation at exact rate every tick
	//FRotator Orientation = FMath::RInterpTo(CurrentOrientation, TargetOrientation, 0.01f, 1.f);
	//MeshParent->SetRelativeRotation(TargetOrientation);
}

void ABoid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Ensure the timer is cleared by using the timer handle
	GetWorld()->GetTimerManager().ClearTimer(FT_Handle_AutoOrient);

	// Alternatively you can clear all timers that belong to this (Actor) instance.
	//GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}