// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemySpawnerComponent.h"
#include "RoomLayout.h"
#include "GameFramework/Actor.h"
#include "RoomPool.h"
#include "Core/ScrapItGameInstance.h"
#include "Scraps/ScrapLootTable.h"
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
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float SpawnInterval = 0.15f;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	UScrapLootTable* LootTable;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float DoorOffset = 500.f;
	
	UPROPERTY(EditAnywhere, Category = "Room Generation")
	TArray<TSubclassOf<ARoomLayout>> RoomLayouts;
	
	UPROPERTY(EditAnywhere, Category = "Room Generation")
	TArray<TSubclassOf<AActor>> ObstaclePool;
	
	UPROPERTY()
	UEnemySpawnerComponent* EnemySpawner;
	
	//Room State
	UPROPERTY()
	ARoomLayout* CurrentRoomLayout;
	
	UPROPERTY(VisibleAnywhere, Category = "Room State")
	ERoomState RoomState = ERoomState::Active;
	
	UPROPERTY(VisibleAnywhere, Category = "Room State")
	int32 CurrentRoomRank = 1;
	
	UPROPERTY()
	UScrapItGameInstance* GameInstance;
	
	//Functions
	void SpawnRoomLayout();
	void ApplyRoomModifiers();
	
	UFUNCTION()
	void HandleEnemyLoot(FVector Location, int32 BaseDropAmount);
	
	UFUNCTION()
	void CompleteRoom();
	
	void SpawnDoors();

public:
	FOnRoomCompleted OnRoomCompleted;
};
