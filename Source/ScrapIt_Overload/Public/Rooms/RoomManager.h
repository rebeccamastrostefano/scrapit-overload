// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyPool.h"
#include "GameFramework/Actor.h"
#include "RoomPool.h"
#include "RoomManager.generated.h"

UENUM()
enum ERoomModifiers : uint8
{
	EnemyBoost,
	OilHazard,
	COUNT
};

UENUM(BlueprintType)
enum class ERoomState : uint8
{
	Active,
	Completed,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoomCompleted);

UCLASS()
class SCRAPIT_OVERLOAD_API ARoomManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Room Config
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	ERoomType RoomType;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	int32 BaseEnemyCount = 5;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float MinSpawnDistance = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float MaxSpawnDistance = 2500.f;
	
	//Functions
	void ApplyRoomModifiers();
	void SpawnEnemies();
	FVector GetRandomSpawnPoint();
	void RegisterEnemy(AActor* Enemy);
	void SpawnRandomScrapsAtLocation(FVector Location, int32 Amount);
	ERoomType GetRandomRoomType();
	void CompleteRoom();
	
	//Data
	UPROPERTY(VisibleAnywhere, Category = "Room State")
	ERoomState RoomState = ERoomState::Active;
	
	UPROPERTY(VisibleAnywhere, Category = "Room State")
	int32 CurrentRoomRank = 1;
	
	UPROPERTY()
	UEnemyPool* ActiveEnemyPool;
	
	int32 EnemiesToSpawn = 0;
	int32 EnemyCount = 0;
	FTimerHandle SpawnTimerHandle;
	

public:	
	//Objectives handling
	UFUNCTION()
	void OnEnemyDeath(FVector Location, int32 ScrapsToSpawn);
	
	//Events
	FOnRoomCompleted OnRoomCompleted;
};
