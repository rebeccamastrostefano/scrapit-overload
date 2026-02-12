// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/EnemyPool.h"

FEnemyDetails UEnemyPool::GetRandomEnemyBasedOnChance()
{
	const float TotalChances = Algo::Accumulate(Enemies, 0.f, [](float Sum, const FEnemyDetails& Details)
	{
		return Sum + Details.SpawnChance;
	});
	
	if (TotalChances > 0.f)
	{
		const float Random = FMath::FRandRange(0.f, TotalChances);
		float ChanceSum = 0.f;
		
		for (const FEnemyDetails& Details : Enemies)
		{
			ChanceSum += Details.SpawnChance;
			if (Random <= ChanceSum)
			{
				return Details;
			}
		}
	}
	
	return Enemies.Num() > 0 ? Enemies[0] : FEnemyDetails();
}
