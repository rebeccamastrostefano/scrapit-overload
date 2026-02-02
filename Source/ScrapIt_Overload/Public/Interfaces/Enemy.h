// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Rooms/RoomManager.h"
#include "Enemy.generated.h"

DECLARE_DELEGATE_TwoParams(FOnEnemyDeath, FVector, int32);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemy : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SCRAPIT_OVERLOAD_API IEnemy
{
	GENERATED_BODY()

public:
	float BaseHealth;
	float Damage;
	int32 ScrapDrop;
	virtual void RegisterToRoomManager(ARoomManager* RoomManager) = 0;
};
