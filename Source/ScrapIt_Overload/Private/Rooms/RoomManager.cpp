// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
#include "Rooms/Door.h"
#include "Scraps/ScrapActor.h"
#include "Interfaces/Enemy.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"

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
	
	if (UPersistentManager* PM = GetGameInstance()->GetSubsystem<UPersistentManager>())
	{
		if (UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance()))
		{
			CurrentRoomRank = PM->GetRoomRank();
			ActiveEnemyPool = GI->GetEnemyPool(CurrentRoomRank);
		}
	}
	
	if (RoomType == ERoomType::Standard)
	{
		ApplyRoomModifiers();
		SpawnEnemies();
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
			//More enemies
			EnemiesToSpawn *= CurrentRoomRank;
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
void ARoomManager::SpawnEnemies()
{
	if (!ActiveEnemyPool || ActiveEnemyPool->Enemies.Num() == 0)
	{
		return;
	}
	
	for (int32 i = 0; i < EnemiesToSpawn; i++)
	{
		FEnemyDetails EnemyDetails = ActiveEnemyPool->GetRandomEnemyBasedOnChance();
		FVector SpawnLocation = GetRandomSpawnPoint();
		
		for (int32 e = 0; e < EnemyDetails.ClusterCount; e++)
		{
			FVector SpawnLoc = SpawnLocation + (FMath::VRand() * 100.f);
			SpawnLoc.Z = 100.f;
			
			AActor* Enemy = GetWorld()->SpawnActor<AActor>(EnemyDetails.EnemyClass, SpawnLoc, FRotator::ZeroRotator);
			RegisterEnemy(Enemy);
			EnemyCount++;
		}
	}
}

FVector ARoomManager::GetRandomSpawnPoint()
{
	APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();
	
	if (!Player)
	{
		return FVector::ZeroVector;
	}
	
	FVector RandomDir = FMath::VRand();
	RandomDir.Z = 0.f;
	RandomDir.Normalize();
	
	float Distance = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
	return Player->GetActorLocation() + (RandomDir * Distance);
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
void ARoomManager::OnEnemyDeath(FVector Location, int32 ScrapsToSpawn)
{
	if (RoomType == ERoomType::Standard)
	{
		EnemyCount--;
		if (EnemyCount <= 0 && RoomState != ERoomState::Completed)
		{
			CompleteRoom();
		}
	}
	SpawnRandomScrapsAtLocation(Location, ScrapsToSpawn);
}

void ARoomManager::CompleteRoom()
{
	RoomState = ERoomState::Completed;
	OnRoomCompleted.Broadcast();
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	
	const UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance());
	
	if (const TSubclassOf<AActor> DoorBP = GI->DoorBP)
	{
		//Spawn the three doors at the correct positions
		float DoorOffset = -500.f;
		for (int32 i = 0; i < 3; i++)
		{
			FVector SpawnLoc = FVector(GetActorLocation().X, DoorOffset, GetActorLocation().Z);
			if (ADoor* Door = Cast<ADoor>(GetWorld()->SpawnActor<AActor>(DoorBP, SpawnLoc, FRotator::ZeroRotator)))
			{
				Door->SetRoomType(GetRandomRoomType());
				UE_LOG(LogTemp, Warning, TEXT("Spawning door at %s"), *SpawnLoc.ToString());
			}
			DoorOffset += 500.f;
		}
	}
}

//Scrap Spawning
void ARoomManager::SpawnRandomScrapsAtLocation(FVector Location, int32 Amount)
{
	//Safety Checks
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	const UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: GameInstance is NOT UScrapItGameInstance!"));
		return;
	}
	

	TSubclassOf<AActor> const BasicScrapBlueprint = GI->ScrapTypeToBP[EScrapType::Basic];
	if (!BasicScrapBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("ScrapSubsystem: Missing Scrap Blueprint"));
		return;
	}
	
	//Spawn Scraps
	UE_LOG(LogTemp, Warning, TEXT("Attempting Spawning %d scraps"), Amount);
		
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	for (int i = 0; i < Amount; i++)
	{
		//Spread scraps randomly and add a slight lift on Z so they don't spawn inside the floor
		FVector RandomOffset = FMath::VRand() * 50.f;
		RandomOffset.Z = 0.f;
		const FVector FinalLocation = Location + RandomOffset + FVector(0, 0, 40.f);
		
		const float RandomRotation = FMath::RandRange(0.f, 360.f);
		const FRotator Rotation = FRotator(RandomRotation, RandomRotation, RandomRotation);
		
		if (AScrapActor* NewScrap = World->SpawnActor<AScrapActor>(BasicScrapBlueprint, FinalLocation, Rotation, SpawnParams))
		{
			//TODO: randomize scrap type
			NewScrap->SetScrapType(EScrapType::Basic);
			
			//Slight push up for pop effect
			if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(NewScrap->GetRootComponent()))
			{
				FVector RandomImpulse = FMath::VRand() * 300.f;
				Root->AddImpulse(RandomImpulse + FVector(0,0,500), NAME_None, true);
			}
		}
	}
}

ERoomType ARoomManager::GetRandomRoomType()
{
	if (UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance()))
	{
		if (GI->RoomPool && GI->RoomPool->Rooms.Num() > 0)
		{
			float TotalWeight = 0.f;
			for (const auto& Room : GI->RoomPool->Rooms)
			{
				TotalWeight += Room.Weight;
			}
			
			const float RandomValue = FMath::RandRange(0.f, TotalWeight);
			float WeightSum = 0.f;
			
			for (const auto& Room : GI->RoomPool->Rooms)
			{
				WeightSum += Room.Weight;
				if (RandomValue <= WeightSum)
				{
					return Room.Room;
				}
			}
		}
	}
	return ERoomType::Standard;
}

