// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/Enemy_BoltTick.h"
#include "Scraps/ScrapSubsystem.h"

// Sets default values
AEnemy_BoltTick::AEnemy_BoltTick()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoltTickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoltTickMesh"));
	RootComponent = BoltTickMesh;
	
	HurtboxSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HurtboxSphere"));
	HurtboxSphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEnemy_BoltTick::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = BaseHealth;
}

// Called every frame
void AEnemy_BoltTick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy_BoltTick::TakeDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	
	if (CurrentHealth <= 0)
	{
		SpawnScrap();
		Destroy();
	}
}

void AEnemy_BoltTick::SpawnScrap()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning %d scraps"), ScrapDrop);
	GetWorld()->GetSubsystem<UScrapSubsystem>()->SpawnRandomScrapsAtLocation(GetActorLocation(), ScrapDrop);
}

