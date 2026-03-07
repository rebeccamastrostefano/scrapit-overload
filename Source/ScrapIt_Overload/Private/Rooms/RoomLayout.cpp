// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/RoomLayout.h"

#include "Core/ScrapItGameInstance.h"
#include "Rooms/Door.h"
#include "Rooms/RoomManager.h"

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

void ARoomLayout::SpawnDoorAtSocket(const FName SocketTag, const int32 RoomID)
{
	//Get door location from a random door tag
	const TArray<UActorComponent*> DoorSpawnPoints = GetComponentsByTag(USceneComponent::StaticClass(), SocketTag);

	if (DoorSpawnPoints.Num() > 0)
	{
		const USceneComponent* SpawnPoint = Cast<USceneComponent>(DoorSpawnPoints[0]);
		const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());

		check(GameInstance != nullptr);

		if (const TSubclassOf<AActor> DoorBP = GameInstance->DoorBP)
		{
			//Spawn door
			if (ADoor* Door = GetWorld()->SpawnActor<ADoor>(DoorBP, SpawnPoint->GetComponentLocation(),
			                                                FRotator::ZeroRotator))
			{
				Door->SetRoomID(RoomID);

				EDoorDirection Direction = None;
				if (SocketTag == ARoomManager::GetDoorSocketName(North)) Direction = North;
				else if (SocketTag == ARoomManager::GetDoorSocketName(South)) Direction = South;
				else if (SocketTag == ARoomManager::GetDoorSocketName(East)) Direction = East;
				else if (SocketTag == ARoomManager::GetDoorSocketName(West)) Direction = West;
				Door->SetDoorDirection(Direction);

				Doors.Add(Door);
			}
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

USceneComponent* ARoomLayout::GetDoorAtSocket(const FName SocketTag) const
{
	const TArray<UActorComponent*> DoorSockets = GetComponentsByTag(USceneComponent::StaticClass(), SocketTag);

	if (DoorSockets.Num() > 0)
	{
		return Cast<USceneComponent>(DoorSockets[0]);
	}

	return nullptr;
}

void ARoomLayout::SetDoorsState(const bool bAreDoorsOpen)
{
	if (bAreDoorsOpen)
	{
		for (ADoor* Door : Doors)
		{
			Door->Open();
		}
	}
	else
	{
		for (ADoor* Door : Doors)
		{
			Door->Close();
		}
	}
}
