// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBoltTick.h"

#include "AIController.h"
#include "Mecha/MechaPawn.h"

// Sets default values
AEnemyBoltTick::AEnemyBoltTick()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	
	HurtboxSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HurtboxSphere"));
	HurtboxSphere->SetupAttachment(RootComponent);
	
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement Component"));
	PawnMovement->MaxSpeed = MoveSpeed;
}

// Called when the game starts or when spawned
void AEnemyBoltTick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentState = EEnemyState::Chasing;
}

void AEnemyBoltTick::Tick(float DeltaTime)
{
	if (CurrentState == EEnemyState::Chasing && Player != nullptr)
	{
		const float DistanceToPlayer = FVector::Dist(Player->GetActorLocation(), GetActorLocation());
		if (DistanceToPlayer <= AttackRange)
		{
			if (IsFacingPlayer())
			{
				Attack();
			}
			else
			{
				//This generally would not happen, so it's okay to snap the rotation with no interpolation
				const FVector DirectionToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				SetActorRotation(DirectionToPlayer.Rotation());
			}
		}
	}
}

void AEnemyBoltTick::Attack()
{
	if (CurrentState == EEnemyState::Dead || AIController == nullptr)
	{
		return;
	}
	
	AIController->StopMovement();
	SetState(EEnemyState::Attacking);
}