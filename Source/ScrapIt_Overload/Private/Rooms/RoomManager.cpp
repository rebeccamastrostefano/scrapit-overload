// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Scraps/ScrapFactory.h"

// Sets default values
ARoomManager::ARoomManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemySpawner = CreateDefaultSubobject<UEnemySpawnerComponent>(TEXT("Enemy Spawner"));
}

// Called when the game starts or when spawned
void ARoomManager::BeginPlay()
{
	Super::BeginPlay();

	PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	check(PersistentManager != nullptr);

	GameInstance = GetGameInstance<UScrapItGameInstance>();
	check(GameInstance != nullptr);

	LevelsManager = GetGameInstance()->GetSubsystem<ULevelsManager>();
	check(LevelsManager != nullptr);

	InitializeRoom();
}

void ARoomManager::InitializeRoom()
{
	RoomID = LevelsManager->GetCurrentRoomID();
	if (LevelsManager->GetLevelMap().Contains(RoomID))
	{
		FRoomNode& RoomNode = LevelsManager->GetLevelMap()[RoomID];

		//Spawn doors according to connections
		SpawnDoors(RoomNode);

		if (RoomNode.bIsVisited)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARoomManager::CompleteRoom);
		}
	}

	//Init Room Layout (assigned in level)
	if (CurrentRoomLayout == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Room Layout is NULL!"));
		return;
	}

	CurrentRoomLayout->GenerateObstacles(ObstaclePool);
	EnemySpawner->SetRoomLayout(CurrentRoomLayout);


	//if the room is a combat room, handle enemies
	if (RoomType == ERoomType::Combat)
	{
		//Register to Enemy Spawner Events
		EnemySpawner->OnEnemyEliminated.AddDynamic(this, &ARoomManager::HandleEnemyLoot);
		EnemySpawner->OnEnemiesCleared.AddDynamic(this, &ARoomManager::CompleteRoom);

		CurrentLevelRank = PersistentManager->GetRoomRank();
		UEnemyPool* Pool = GameInstance->GetEnemyPool(CurrentLevelRank);

		//Scale up the enemy count based on level rank
		BaseEnemyCount *= CurrentLevelRank;

		//Apply modifiers
		ApplyRoomModifiers();

		//Spawn enemies in the room
		if (Pool != nullptr)
		{
			EnemySpawner->RequestSpawnWave(Pool, BaseEnemyCount, SpawnInterval);
		}
	}
}

void ARoomManager::SpawnDoors(FRoomNode& RoomNode) const
{
	for (const int32 NeighborID : RoomNode.ConnectedRoomsIDs)
	{
		const FIntPoint NeighborPosition = LevelsManager->GetLevelMap()[NeighborID].Coordinates;
		const FIntPoint Difference = NeighborPosition - RoomNode.Coordinates;

		FName TargetSocket = NAME_None;
		if (Difference == FIntPoint(0, 1))
		{
			TargetSocket = "Door_N";
		}
		else if (Difference == FIntPoint(0, -1))
		{
			TargetSocket = "Door_S";
		}
		else if (Difference == FIntPoint(1, 0))
		{
			TargetSocket = "Door_E";
		}
		else if (Difference == FIntPoint(-1, 0))
		{
			TargetSocket = "Door_W";
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RoomManager: Invalid door connection!"))
		}

		CurrentRoomLayout->SpawnDoorAtSocket(TargetSocket, NeighborID);
	}
}

void ARoomManager::ApplyRoomModifiers()
{
	//The higher the level rank, the higher the chance to get a modifier (impossible on first two ranks)
	if (FMath::RandRange(40, 180) < (20 * CurrentLevelRank))
	{
		const ERoomModifiers Modifier = static_cast<ERoomModifiers>(FMath::RandRange(0, COUNT - 1));
		switch (Modifier)
		{
		case EnemyBoost:
			//Double the enemies
			BaseEnemyCount *= 2;
			break;
		case OilHazard:
			//TODO: Spawn oil puddles
			break;
		default:
			break;
		}
	}
}

//Enemy Death
void ARoomManager::HandleEnemyLoot(FVector Location, int32 BaseDropAmount)
{
	if (LootTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Scrap Loot Table is NULL!"));
		return;
	}

	//Spawn Scraps at the location of the enemy death
	UScrapFactory::SpawnScraps(this, Location, LootTable, BaseDropAmount);
}

//Objective Management
void ARoomManager::CompleteRoom()
{
	RoomState = ERoomState::Completed;
	if (LevelsManager != nullptr && LevelsManager->GetLevelMap().Contains(RoomID))
	{
		LevelsManager->MarkRoomAsVisited(RoomID);

		//Spawn Doors for neighbor doors
		TArray<int32> Connections = LevelsManager->GetLevelMap()[RoomID].ConnectedRoomsIDs;
		for (const int32 Connection : Connections)
		{
			FName SocketTag = FName(*FString::Printf(TEXT("Door_%d"), Connection));
			CurrentRoomLayout->SpawnDoorAtSocket(SocketTag, Connection);
		}
	}

	OnRoomCompleted.Broadcast();
}
