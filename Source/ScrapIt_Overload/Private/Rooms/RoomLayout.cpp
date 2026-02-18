// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/RoomLayout.h"

#include "Core/ScrapItGameInstance.h"

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

void ARoomLayout::SpawnDoor() const
{
	//Get door location from a random door tag
	const TArray<UActorComponent*> DoorSpawnPoints = GetComponentsByTag(USceneComponent::StaticClass(), "Door");
	
	if (DoorSpawnPoints.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, DoorSpawnPoints.Num() - 1);
		const FVector SpawnLocation = Cast<USceneComponent>(DoorSpawnPoints[RandomIndex])->GetComponentLocation();
		
		const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
		if (GameInstance == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("RoomLayout: GameInstance is NOT UScrapItGameInstance!"));
			return;
		}
		
		if (const TSubclassOf<AActor> DoorBP = GameInstance->DoorBP)
		{
			//Spawn door
			GetWorld()->SpawnActor<AActor>(DoorBP, SpawnLocation, FRotator::ZeroRotator);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RoomLayout: No Door BP assigned in GameInstance"));
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

