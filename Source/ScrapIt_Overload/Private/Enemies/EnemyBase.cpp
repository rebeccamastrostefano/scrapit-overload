// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemyBase::Die()
{
	OnDeath.Broadcast(GetActorLocation(), BaseDropAmount);
	Destroy();
}

