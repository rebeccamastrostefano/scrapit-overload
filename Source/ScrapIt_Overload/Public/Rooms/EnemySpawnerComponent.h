// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyPool.h"
#include "EnemySpawnerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemiesCleared);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyEliminated, FVector, DeathLocation, int32, BaseDropAmount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCRAPIT_OVERLOAD_API UEnemySpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemySpawnerComponent();

protected:
	void ProcessSpawnQueue();
	void SpawnEnemyCluster();

	// Helpers
	FVector GetRandomSpawnPoint() const;
	FVector GetRandomClusterMemberSpawnPoint(const FVector& Center) const;
	
	UFUNCTION()
	void OnEnemyDeath(FVector Location, int32 BaseDropAmount);

	UPROPERTY()
	UEnemyPool* ActivePool;

	FTimerHandle SpawnQueueTimerHandle;
	int32 PendingClusters = 0;
	int32 ActiveEnemyCount = 0;
	float SpawnInterval = 0.15f;

	// Spawn config (could be moved to variables or a DataAsset)
	float MinSpawnDistance = 800.f;
	float MaxSpawnDistance = 1500.f;

public:	
	UPROPERTY()
	UEnemyPool* ActiveEnemyPool;

	void RequestSpawnWave(UEnemyPool* Pool, const int32 TotalClusters, float Interval);

	//Events
	UPROPERTY(BlueprintAssignable)
	FOnEnemiesCleared OnEnemiesCleared;

	UPROPERTY(BlueprintAssignable)
	FOnEnemyEliminated OnEnemyEliminated;
};
