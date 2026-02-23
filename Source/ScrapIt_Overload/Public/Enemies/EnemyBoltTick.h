// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.h"
#include "EnemyBoltTick.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemyBoltTick : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBoltTick();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float AttackRange = 100.f;
	
public:	
	virtual void Attack() override;
};
