// Fill out your copyright notice in the Description page of Project Settings.


#include "Scraps/ScrapBase.h"

// Sets default values
AScrapBase::AScrapBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	ScrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = ScrapMesh;
	
	ScrapMesh->SetSimulatePhysics(true);
	ScrapMesh->SetLinearDamping(5.0f);
}

// Called every frame
void AScrapBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentState == EScrapState::Idle)
	{
		return;
	}
	
	//If magnet has stopped pulling, release scrap
	float const TimeSinceLastPull = GetWorld()->GetTimeSeconds() - LastPullTime;
	if (TimeSinceLastPull > MagnetTimeout)
	{
		OnMagnetReleased();
		return;
	}
	
	FVector const CurrentLocation = GetActorLocation();
	
	//If magnet is active, scrap will rise and then get pulled towards the mecha
	if (CurrentState == EScrapState::Rising)
	{
		RiseUp(CurrentLocation, DeltaTime);
	}
	else if (CurrentState == EScrapState::Pulling)
	{
		ApplyPullForce(CurrentLocation, DeltaTime);
	}
}

void AScrapBase::OnMagnetPulled(AActor* MechaActor, float const PullStrength, float const PullRadius, float const CollectionRadius)
{
	LastPullTime = GetWorld()->GetTimeSeconds();
	
	//If we are not already pulling, start pull
	if (CurrentState != EScrapState::Idle)
	{
		return;
	}
	
	SetActorTickEnabled(true);
	PullingActor = MechaActor;
	MagnetStrength = PullStrength;
	MagnetRadius = PullRadius;
	CollectionDistance = CollectionRadius;
	
	if (ScrapMesh->IsSimulatingPhysics())
	{
		ScrapMesh->SetSimulatePhysics(false);
	}
	TargetHoverLocation = GetActorLocation() + FVector(0.0f, 0.0f, HoverHeight);
	CurrentState = EScrapState::Rising;
}

void AScrapBase::RiseUp(const FVector& CurrentLocation, const float DeltaTime)
{
	//Move up to hover location
	FVector const NewLocation = FMath::VInterpTo(CurrentLocation, TargetHoverLocation, DeltaTime, 2.0f);
	SetActorLocation(NewLocation);
		
	if (FVector::DistSquared(CurrentLocation, TargetHoverLocation) > 100.0f)
	{
		//If the scrap has risen, start moving towards mecha
		CurrentState = EScrapState::Pulling;
	}
}

void AScrapBase::ApplyPullForce(const FVector& CurrentLocation, const float DeltaTime)
{
	if (PullingActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScrapActor: PullingActor is NULL!"));
		return;
	}
	
	//Move towards the mecha
	FVector const TargetLocation = PullingActor->GetActorLocation() + FVector(0.0f, 0.0f, HoverHeight);
	float const Distance = FVector::Dist(CurrentLocation, TargetLocation);
		
	//Increase speed the closer the scrap is to the mecha
	float const DistanceAlpha = FMath::Clamp(1.0f - (Distance / MagnetRadius), 0.0f, 1.0f);
	float const CurrentSpeed = BasePullSpeed * (MaxBoostSpeed * FMath::Pow(DistanceAlpha, 2.0f));
		
	FVector const NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MagnetStrength * CurrentSpeed);
	SetActorLocation(NewLocation);
		
	if (FVector::Dist(CurrentLocation, TargetLocation) < CollectionDistance)
	{
		//If close to the mecha, scrap can be collected
		OnCollected();
	}
}

void AScrapBase::OnMagnetReleased()
{
	if (!ScrapMesh->IsSimulatingPhysics())
	{
		ScrapMesh->SetSimulatePhysics(true);
	}
	SetActorTickEnabled(false);
	CurrentState = EScrapState::Idle;
}

