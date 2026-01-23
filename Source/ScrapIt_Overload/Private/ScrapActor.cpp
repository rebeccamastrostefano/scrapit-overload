// Fill out your copyright notice in the Description page of Project Settings.


#include "ScrapActor.h"
#include "MechaPawn.h"

// Sets default values
AScrapActor::AScrapActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ScrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = ScrapMesh;
	
	ScrapMesh->SetSimulatePhysics(true);
	ScrapMesh->SetLinearDamping(5.0f);
}

// Called when the game starts or when spawned
void AScrapActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AScrapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentState == EScrapState::Idle)
	{
		return;
	}
	
	float const TimeSinceLastPull = GetWorld()->GetTimeSeconds() - LastPullTime;
	if (TimeSinceLastPull > MagnetTimeout)
	{
		OnMagnetReleased();
		return;
	}
	
	FVector const CurrentLocation = GetActorLocation();
	
	if (CurrentState == EScrapState::Rising)
	{
		FVector const NewLocation = FMath::VInterpTo(CurrentLocation, TargetHoverLocation, DeltaTime, 2.0f);
		SetActorLocation(NewLocation);
		
		if (FVector::DistSquared(CurrentLocation, TargetHoverLocation) > 100.0f)
		{
			CurrentState = EScrapState::Pulling;
		}
	}
	else if (CurrentState == EScrapState::Pulling)
	{
		if (PullingActor)
		{
			FVector const TargetLocation = PullingActor->GetActorLocation() + FVector(0.0f, 0.0f, HoverHeight);
			float const Distance = FVector::Dist(CurrentLocation, TargetLocation);
			
			//Increase speed the closer the scrap is to the mecha
			float const DistanceAlpha = FMath::Clamp(1.0f - (Distance / MagnetRadius), 0.0f, 1.0f);
			float const CurrentSpeed = BasePullSpeed * (MaxBoostSpeed * FMath::Pow(DistanceAlpha, 2.0f));
			
			FVector const NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MagnetStrength * CurrentSpeed);
			SetActorLocation(NewLocation);
			
			if (FVector::Dist(CurrentLocation, TargetLocation) < CollectionDistance)
			{
				OnCollected();
			}
		}
	}
}

void AScrapActor::OnMagnetPulled(AActor* MechaActor, float const PullStrength, float const PullRadius, float const CollectionRadius)
{
	if (CurrentState != EScrapState::Idle)
	{
		return;
	}

	LastPullTime = GetWorld()->GetTimeSeconds();
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

void AScrapActor::OnMagnetReleased()
{
	if (!ScrapMesh->IsSimulatingPhysics())
	{
		ScrapMesh->SetSimulatePhysics(true);
	}
	CurrentState = EScrapState::Idle;
}

void AScrapActor::OnCollected()
{
	if (AMechaPawn* Mecha = Cast<AMechaPawn>(PullingActor))
	{
		Mecha->AddScrap(1);
	}
	Destroy();
}

