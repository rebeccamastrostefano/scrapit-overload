// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Rooms/Objectives/RoomObjective.h"
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
	//Spawn a random room layout
	const TSubclassOf<ARoomLayout> RoomLayoutClass = RoomLayoutPool[FMath::RandRange(0, RoomLayoutPool.Num() - 1)];
	CurrentRoomLayout = GetWorld()->SpawnActor<ARoomLayout>(RoomLayoutClass);

	if (CurrentRoomLayout == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Room Layout actor NOT FOUND in the level!"));
		return;
	}

	RoomID = LevelsManager->GetCurrentRoomID();
	if (LevelsManager->GetLevelMap().Contains(RoomID))
	{
		FRoomNode& RoomNode = LevelsManager->GetLevelMap()[RoomID];

		//Spawn doors according to connections
		SpawnDoors(RoomNode);

		//If the room was already visited, complete it
		if (RoomNode.bIsVisited || RoomType == ERoomType::Exit)
		{
			CompleteRoom();
		}
		else
		{
			SetupObjective();
			CurrentRoomLayout->SetDoorsState(false);
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
	TeleportPlayerToEntry();

	//if the room is a combat room and has not been already visited, handle enemies
	if (RoomType == ERoomType::Combat && RoomState != ERoomState::Completed)
	{
		InitializeCombat();
	}
}

void ARoomManager::SetupObjective()
{
	if (GameInstance->RoomObjectives.Contains(RoomType))
	{
		CurrentObjective = NewObject<URoomObjective>(this, GameInstance->RoomObjectives[RoomType]);
		CurrentObjective->OnObjectiveFinished.AddDynamic(this, &ARoomManager::CompleteRoom);
		CurrentObjective->ActivateObjective(this);
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
			TargetSocket = GetDoorSocketName(North);
		}
		else if (Difference == FIntPoint(0, -1))
		{
			TargetSocket = GetDoorSocketName(South);
		}
		else if (Difference == FIntPoint(1, 0))
		{
			TargetSocket = GetDoorSocketName(East);
		}
		else if (Difference == FIntPoint(-1, 0))
		{
			TargetSocket = GetDoorSocketName(West);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RoomManager: Invalid door connection!"))
		}

		CurrentRoomLayout->SpawnDoorAtSocket(TargetSocket, NeighborID);
	}
}

void ARoomManager::InitializeCombat()
{
	//Register to Enemy Spawner Events
	EnemySpawner->OnEnemyEliminated.AddDynamic(this, &ARoomManager::HandleEnemyLoot);

	CurrentLevelRank = PersistentManager->GetLevelRank();
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

void ARoomManager::TeleportPlayerToEntry() const
{
	const EDoorDirection EntryDirection = LevelsManager->
		GetOppositeDoorDirection(LevelsManager->GetLastExitDirection());
	if (EntryDirection == None || RoomType == ERoomType::Start)
	{
		return; //Likely the starting room, player should already be in position with the PlayerStart
	}

	USceneComponent* EntryPoint = CurrentRoomLayout->GetDoorAtSocket(GetDoorSocketName(EntryDirection));
	if (EntryPoint != nullptr)
	{
		AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (Player != nullptr && EntryPoint != nullptr)
		{
			const FVector SpawnLocation = EntryPoint->GetComponentLocation() + (EntryPoint->GetForwardVector() * 300.f);
			Player->SetActorLocationAndRotation(SpawnLocation, EntryPoint->GetComponentRotation());
		}
		UE_LOG(LogTemp, Warning, TEXT("RoomManager: Teleported player to room entry point!"))
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Entry point socket not found!"))
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

		CurrentRoomLayout->SetDoorsState(true);
	}

	OnRoomCompleted.Broadcast();
}
