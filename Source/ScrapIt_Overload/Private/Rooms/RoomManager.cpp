// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/RoomManager.h"
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
	
	if (UPersistentManager* PM = GetGameInstance()->GetSubsystem<UPersistentManager>())
	{
		if (UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance()))
		{
			const int32 RoomRank = PM->GetRoomRank();
			ActiveEnemyPool = GI->GetEnemyPool(RoomRank);
		}
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ARoomManager::SpawnCycle, 2.f, true);
}

// Called every frame
void ARoomManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Enemy Spawning
void ARoomManager::SpawnCycle()
{
	if (!ActiveEnemyPool || ActiveEnemyPool->Enemies.Num() == 0)
	{
		return;
	}
	
	FEnemyDetails EnemyDetails = ActiveEnemyPool->GetRandomEnemyBasedOnChance();
	FVector SpawnLocation = GetRandomSpawnPoint();
	
	for (int32 i = 0; i < EnemyDetails.ClusterCount; i++)
	{
		FVector SpawnLoc = SpawnLocation + (FMath::VRand() * 100.f);
		SpawnLoc.Z = 100.f;
		
		AActor* Enemy = GetWorld()->SpawnActor<AActor>(EnemyDetails.EnemyClass, SpawnLoc, FRotator::ZeroRotator);
		RegisterEnemy(Enemy);
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

//Scrap Spawning
void ARoomManager::SpawnRandomScrapsAtLocation(FVector Location, int32 Amount)
{
	//Safety Checks
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomManager: GameInstance is NOT UScrapItGameInstance!"));
		return;
	}
	

	TSubclassOf<AActor> const BasicScrapBlueprint = GI->BasicScrapBP;
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
		
		const float RandomYaw = FMath::RandRange(0.f, 360.f);
		const FRotator RandomRotation = FRotator(0.f, RandomYaw, 0.f);
		
		if (AScrapActor* NewScrap = World->SpawnActor<AScrapActor>(BasicScrapBlueprint, FinalLocation, RandomRotation, SpawnParams))
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

