// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/EnemyPool.h"

FEnemyDetails UEnemyPool::GetRandomEnemyBasedOnChance()
{
	float TotalChances = 0.f;
	for (const FEnemyDetails& Details : Enemies)
	{
		TotalChances += Details.SpawnChance;
	}
	
	float Random = FMath::FRandRange(0.f, TotalChances);
	float ChanceSum = 0.f;
	for (const FEnemyDetails& Details : Enemies)
	{
		ChanceSum += Details.SpawnChance;
		if (Random <= ChanceSum)
		{
			return Details;
		}
	}
	
	return Enemies[0];
}
