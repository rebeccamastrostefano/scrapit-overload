// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/RoomLayout.h"

// Sets default values
ARoomLayout::ARoomLayout()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARoomLayout::BeginPlay()
{
	Super::BeginPlay();
	
	//Populate arrays finding the tags
	TArray<UActorComponent*> FoundComponents = GetComponentsByTag(UBoxComponent::StaticClass(), "SpawnZone");
	for (UActorComponent* Comp : FoundComponents)
	{
		SpawnZones.Add(Cast<UBoxComponent>(Comp));
	}
	
	FoundComponents = GetComponentsByTag(USceneComponent::StaticClass(), "ObstacleSlot");
	for (UActorComponent* Comp : FoundComponents)
	{
		ObstacleSlots.Add(Cast<USceneComponent>(Comp));
	}
}

void ARoomLayout::GenerateObstacles(TArray<TSubclassOf<AActor>> ObstaclePool)
{
	if (ObstaclePool.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoomLayout: No obstacles in pool"));
		return;
	}
	
	for (const USceneComponent* Slot : ObstacleSlots)
	{
		//40% chance of spawning and obstacle on slot
		if (FMath::FRand() <= 0.4f)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
			
			const int32 RandomIndex = FMath::RandRange(0, ObstaclePool.Num() - 1);
			GetWorld()->SpawnActor<AActor>(ObstaclePool[RandomIndex], Slot->GetComponentTransform(), Params);
		}
	}
}

UBoxComponent* ARoomLayout::GetRandomSpawnZone() const
{
	if (SpawnZones.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoomLayout: No spawn zones"));
		return nullptr;
	}
	const int32 RandomIndex = FMath::RandRange(0, SpawnZones.Num() - 1);
	return SpawnZones[RandomIndex];
}

