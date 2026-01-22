// Fill out your copyright notice in the Description page of Project Settings.


#include "ScrapActor.h"

// Sets default values
AScrapActor::AScrapActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ScrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = ScrapMesh;
	
	ScrapMesh->SetSimulatePhysics(true);
	ScrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ScrapMesh->SetCollisionObjectType(ECC_PhysicsBody);
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

}

void AScrapActor::OnMagnetPulled(FVector const MagnetLocation, float const PullStrength)
{
	if (ScrapMesh && ScrapMesh->IsSimulatingPhysics())
	{
		FVector Direction = MagnetLocation - GetActorLocation();
		Direction.Normalize();
		
		ScrapMesh->AddForce(Direction * PullStrength, NAME_None, true);
	}
}

