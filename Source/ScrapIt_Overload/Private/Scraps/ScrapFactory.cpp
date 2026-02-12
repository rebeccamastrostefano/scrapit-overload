// Fill out your copyright notice in the Description page of Project Settings.


#include "Scraps/ScrapFactory.h"

void UScrapFactory::SpawnScraps(const UObject* WorldContextObject, const FVector& Location, const UScrapLootTable* LootTable, const int32 BaseDropAmount)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr || LootTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to spawn Scraps: No World or LootTable"));
	}
	
	const int32 SpawnCount = BaseDropAmount * FMath::RandRange(LootTable->MinNumDrops, LootTable->MaxNumDrops);
	for (int32 i = 0; i < SpawnCount; i++)
	{
		for (const FScrapDrop& ScrapDrop : LootTable->PossibleDrops)
		{
			if (FMath::FRand() <= ScrapDrop.DropChance)
			{
				FVector SpawnLocation = Location + (FMath::VRand() * 50.f);
				const float RandomRotation = FMath::RandRange(0.f, 360.f);
				const FRotator Rotation = FRotator(RandomRotation, RandomRotation, RandomRotation);
				
				if (const AScrapBase* NewScrap = World->SpawnActor<AScrapBase>(ScrapDrop.ScrapClass, SpawnLocation, Rotation))
				{
					FinalizeSpawn(NewScrap);
				}
			}
		}
	}
}

void UScrapFactory::FinalizeSpawn(const AScrapBase* NewScrap)
{
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(NewScrap->GetRootComponent()))
	{
		const FVector RandomImpulse = FMath::VRand() * 300.f;
		Root->AddImpulse(RandomImpulse + FVector(0,0,500), NAME_None, true);
	}
}
