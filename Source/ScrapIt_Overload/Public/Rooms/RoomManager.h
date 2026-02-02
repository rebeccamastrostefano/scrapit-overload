// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyPool.h"
#include "GameFramework/Actor.h"
#include "RoomManager.generated.h"

UENUM(BlueprintType)
enum class ERoomObjective : uint8
{
	Survive,
	KillAmount
};

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
	ERoomObjective RoomObjective;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float ObjectiveTarget = 60.f; //Can be seconds or kill amount
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float MinSpawnDistance = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	float MaxSpawnDistance = 2500.f;
	
	UPROPERTY(EditAnywhere, Category = "Room Configuration")
	TMap<int32, UEnemyPool*> RankToEnemyPool;
	
	//Functions
	void SpawnCycle();
	FVector GetRandomSpawnPoint();
	void RegisterEnemy(AActor* Enemy);
	
	//Data
	UPROPERTY()
	UEnemyPool* ActiveEnemyPool;
	
	FTimerHandle SpawnTimerHandle;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Scraps handling
	UFUNCTION()
	void SpawnRandomScrapsAtLocation(FVector Location, int32 Amount);
};
