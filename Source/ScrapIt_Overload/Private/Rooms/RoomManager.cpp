// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
#include "Rooms/Door.h"
#include "Interfaces/Enemy.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Scraps/ScrapFactory.h"

// Sets default values
ARoomManager::ARoomManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoomManager::BeginPlay()
{
	Super::BeginPlay();
	EnemiesToSpawn = BaseEnemyCount;
	
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
	
	CurrentRoomRank = PersistentManager->GetRoomRank();
	ActiveEnemyPool = GameInstance->GetEnemyPool(CurrentRoomRank);
	
	if (RoomType == ERoomType::Standard)
	{
		ApplyRoomModifiers();
		StartSpawnEnemies();
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
			EnemiesToSpawn *= 2;
			break;
		case OilHazard:
			//TODO: Spawn oil puddles
			break;
		default:
			break;
		}
	}
}

//Enemy Spawning
void ARoomManager::StartSpawnEnemies()
{
	if (!ActiveEnemyPool || ActiveEnemyPool->Enemies.Num() <= 0)
	{
		return;
	}
	
	PendingClusters += EnemiesToSpawn;
	
	if (!GetWorldTimerManager().IsTimerActive(SpawnQueueTimerHandle))
	{
		GetWorldTimerManager().SetTimer(SpawnQueueTimerHandle, this, &ARoomManager::ProcessSpawnQueue, SpawnInterval, true);
	}
}

void ARoomManager::ProcessSpawnQueue()
{
	if (PendingClusters <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnQueueTimerHandle);
		return;
	}
	
	SpawnEnemyCluster();
	
	PendingClusters--;
	
	if (PendingClusters <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnQueueTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("All enemies spawned!"));
	}
}

void ARoomManager::SpawnEnemyCluster()
{
	const FEnemyDetails EnemyDetails = ActiveEnemyPool->GetRandomEnemyBasedOnChance();
	if (!EnemyDetails.EnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Enemy Class is NULL!"));
		return;
	}
		
	const FVector CenterLocation = GetRandomSpawnPoint();
		
	for (int32 i = 0; i < EnemyDetails.ClusterCount; i++)
	{
		FVector FinalLocation = GetRandomClusterMemberSpawnPoint(CenterLocation);
			
		if (AActor* Enemy = GetWorld()->SpawnActor<AActor>(EnemyDetails.EnemyClass, FinalLocation, FRotator::ZeroRotator))
		{
			RegisterEnemy(Enemy);
			EnemyCount++;
		}
	}
}

FVector ARoomManager::GetRandomSpawnPoint() const
{
	const APawn* Mecha = GetWorld()->GetFirstPlayerController()->GetPawn();
	
	if (!Mecha)
	{
		return FVector::ZeroVector;
	}
	
	FVector RandomDir = FVector(FMath::RandPointInCircle(150.f), 0.f);
	RandomDir.Normalize();
	
	const float Distance = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
	return Mecha->GetActorLocation() + (RandomDir * Distance);
}

FVector ARoomManager::GetRandomClusterMemberSpawnPoint(const FVector& Center) const
{
	const FVector Offset = FVector(FMath::RandPointInCircle(150.f), 0.f);
	const FVector FinalLocation = Center + Offset;
	
	//Find ground to get Z offset
	FHitResult Hit;
	const FVector StartTrace = FinalLocation + FVector(0, 0, 500.f);
	const FVector EndTrace = FinalLocation - FVector(0, 0, 500.f);
	
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility))
	{
		return Hit.Location + FVector(0, 0, 50.f);
	}
	
	return FinalLocation;
}

void ARoomManager::RegisterEnemy(AActor* Enemy)
{
	if (!Enemy)
	{
		return;
	}
	
	if (IEnemy* EnemyInterface = Cast<IEnemy>(Enemy))
	{
		EnemyInterface->RegisterToRoomManager(this);
	}
}

//Objective Management
void ARoomManager::OnEnemyDeath(FVector Location, int32 BaseDropAmount)
{
	if (RoomType == ERoomType::Standard)
	{
		EnemyCount--;
		if (EnemyCount <= 0 && RoomState != ERoomState::Completed)
		{
			CompleteRoom();
		}
	}
	SpawnRandomScrapsAtLocation(Location, BaseDropAmount);
}

void ARoomManager::CompleteRoom()
{
	RoomState = ERoomState::Completed;
	OnRoomCompleted.Broadcast();
	GetWorldTimerManager().ClearTimer(SpawnQueueTimerHandle);
	
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: GameInstance is NOT UScrapItGameInstance!"));
		return;
	}
	
	if (const TSubclassOf<AActor> DoorBP = GameInstance->DoorBP)
	{
		//Spawn the three doors at the correct positions
		float CurrentDoorOffset = DoorOffset;
		for (int32 i = 0; i < 3; i++)
		{
			FVector SpawnLoc = FVector(GetActorLocation().X, DoorOffset, GetActorLocation().Z);
			if (ADoor* Door = Cast<ADoor>(GetWorld()->SpawnActor<AActor>(DoorBP, SpawnLoc, FRotator::ZeroRotator)))
			{
				Door->SetRoomType(GameInstance->RoomPool->GetRandomRoomType());
			}
			CurrentDoorOffset += DoorOffset;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: No Door BP Assigned!"));
	}
}

//Scrap Spawning
void ARoomManager::SpawnRandomScrapsAtLocation(const FVector Location, const int32 BaseDropAmount) const
{
	if (LootTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: Scrap Loot Table is NULL!"));
		return;
	}
	
	UScrapFactory::SpawnScraps(this, Location, LootTable, BaseDropAmount);
}