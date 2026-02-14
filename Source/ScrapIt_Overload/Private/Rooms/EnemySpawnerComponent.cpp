// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/EnemySpawnerComponent.h"

#include "Enemies/EnemyBase.h"
#include "NavigationSystem.h"

// Sets default values for this component's properties
UEnemySpawnerComponent::UEnemySpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UEnemySpawnerComponent::RequestSpawnWave(UEnemyPool* Pool, const int32 TotalClusters, float Interval)
{
	if (Pool == nullptr || Pool->Enemies.Num() <= 0 || TotalClusters <= 0)
	{
		return;
	}
	
	ActiveEnemyPool = Pool;
	PendingClusters += TotalClusters;
	SpawnInterval = Interval;
	
	if (!GetWorld()->GetTimerManager().IsTimerActive(SpawnQueueTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnQueueTimerHandle, this, &UEnemySpawnerComponent::ProcessSpawnQueue, SpawnInterval, true);
	}
}

void UEnemySpawnerComponent::ProcessSpawnQueue()
{
	if (PendingClusters <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnQueueTimerHandle);
		return;
	}
	
	SpawnEnemyCluster();
	PendingClusters--;
	
	if (PendingClusters <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnQueueTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("All enemies spawned!"));
	}
}

void UEnemySpawnerComponent::SpawnEnemyCluster()
{
	if (ActiveEnemyPool == nullptr)
	{
		return;
	}
	
	const FEnemyDetails EnemyDetails = ActiveEnemyPool->GetRandomEnemyBasedOnChance();
	if (EnemyDetails.EnemyClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemySpawner: Enemy Class is NULL!"));
		return;
	}
	
	const FVector CenterLocation = GetRandomSpawnPoint();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	for (int32 i = 0; i < EnemyDetails.ClusterCount; i++)
	{
		FVector FinalLocation = GetRandomClusterMemberSpawnPoint(CenterLocation);
		
		if (AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyDetails.EnemyClass, FinalLocation, FRotator::ZeroRotator, Params))
		{
			ActiveEnemyCount++;
			Enemy->OnDeath.AddDynamic(this, &UEnemySpawnerComponent::OnEnemyDeath);
		}
	}
}

void UEnemySpawnerComponent::OnEnemyDeath(FVector Location, int32 BaseDropAmount)
{
	ActiveEnemyCount--;
	OnEnemyEliminated.Broadcast(Location, BaseDropAmount);
	
	if (ActiveEnemyCount <= 0 && PendingClusters <= 0)
	{
		OnEnemiesCleared.Broadcast();
	}
}

FVector UEnemySpawnerComponent::GetRandomSpawnPoint() const
{
	if (CurrentRoomLayout == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemySpawner: Room Layout is NULL!"));
		return FVector::ZeroVector;
	}
	
	const UBoxComponent* SelectedZone = CurrentRoomLayout->GetRandomSpawnZone();
	if (SelectedZone == nullptr)
	{
		return FVector::ZeroVector;
	}
	
	//Get radom point inside zone
	const FVector ZoneExtent = SelectedZone->GetScaledBoxExtent();
	const FVector ZoneOrigin = SelectedZone->GetComponentLocation();
	
	const FVector RandomPoint = ZoneOrigin + FVector(
		FMath::RandRange(-ZoneExtent.X, ZoneExtent.X),
		FMath::RandRange(-ZoneExtent.Y, ZoneExtent.Y),
		0.f
	);
	
	return GetValidNavMeshPoint(RandomPoint);
}

FVector UEnemySpawnerComponent::GetValidNavMeshPoint(const FVector& Point) const
{
	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	if (NavigationSystem != nullptr)
	{
		FNavLocation Location;
		if (NavigationSystem->ProjectPointToNavigation(Point, Location, FVector(200.f, 200.f, 500.f)))
		{
			return Location.Location;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemySpawner: Navigation System is NULL!"))
		return Point;
	}
	
	return Point;
}

FVector UEnemySpawnerComponent::GetRandomClusterMemberSpawnPoint(const FVector& Center) const
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

