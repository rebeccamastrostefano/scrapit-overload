// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
#include "Rooms/Door.h"
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
	
	const UPersistentManager* PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	if (PersistentManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: PersistentManager is NULL!"));
	}
	
	GameInstance = GetGameInstance<UScrapItGameInstance>();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: GameInstance is NOT UScrapItGameInstance!"));
		return;
	}
	
	//Register to Enemy Spawner Events
	EnemySpawner->OnEnemyEliminated.AddDynamic(this, &ARoomManager::HandleEnemyLoot);
	EnemySpawner->OnEnemiesCleared.AddDynamic(this, &ARoomManager::CompleteRoom);
	
	CurrentRoomRank = PersistentManager->GetRoomRank();
	UEnemyPool* Pool = GameInstance->GetEnemyPool(CurrentRoomRank);
	
	if (RoomType == ERoomType::Standard)
	{
		//Scale up the enemy count based on room rank
		BaseEnemyCount *= CurrentRoomRank;
		
		//Spawn the room layout and apply modifiers
		SpawnRoomLayout();
		ApplyRoomModifiers();
		
		//Spawn enemies in the room
		if (Pool != nullptr)
		{
			EnemySpawner->RequestSpawnWave(Pool, BaseEnemyCount, SpawnInterval);
		}
	}
}

void ARoomManager::SpawnRoomLayout()
{
	if (RoomLayouts.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: No Room Layouts Found!"));
		return;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, RoomLayouts.Num() - 1);
	CurrentRoomLayout = GetWorld()->SpawnActor<ARoomLayout>(RoomLayouts[RandomIndex], GetActorLocation(), FRotator::ZeroRotator);
	
	if (CurrentRoomLayout != nullptr)
	{
		//Spawn obstacles in room
		CurrentRoomLayout->GenerateObstacles(ObstaclePool);
		EnemySpawner->SetRoomLayout(CurrentRoomLayout);
	}
}

void ARoomManager::ApplyRoomModifiers()
{
	//The higher the room rank, the higher the chance to get a modifier (impossible on first two ranks)
	if (FMath::RandRange(40, 180) < (20 * CurrentRoomRank))
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
	OnRoomCompleted.Broadcast();
	
	if (CurrentRoomLayout != nullptr)
	{
		CurrentRoomLayout->SpawnDoor();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RoomManager: Room Layout is NULL! Door not spawned."));
	}
}
