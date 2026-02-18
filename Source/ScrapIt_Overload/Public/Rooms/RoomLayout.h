// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "RoomLayout.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API ARoomLayout : public AActor
{
	GENERATED_BODY()
	
public:
	ARoomLayout();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<UBoxComponent*> SpawnZones;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<USceneComponent*> ObstacleSlots;
	
public:
	void GenerateObstacles(TArray<TSubclassOf<AActor>> ObstaclePool);
	void SpawnDoor() const;
	
	//Getters
	UBoxComponent* GetRandomSpawnZone() const;

};
