// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameManager.h"

void UGameManager::SpawnRandomScrapsAtLocation(FVector Location, int8 Amount)
{
	if (!BasicScrapBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("ScrapSubsystem: Missing Scrap Blueprint"));
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		for (int i = 0; i < Amount; i++)
		{
			const float RandomYaw = FMath::RandRange(0.f, 360.f);
			const FRotator RandomRotation = FRotator(0.f, RandomYaw, 0.f);
			const FVector FinalLocation = Location + (FMath::VRand() * 50);
			
			AScrapActor* NewScrap = World->SpawnActor<AScrapActor>(BasicScrapBlueprint, FinalLocation, RandomRotation, SpawnParams);
			if (NewScrap)
			{
				NewScrap->SetScrapType(EScrapType::Basic);
				
				//Slight push up for pop effect
				if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(NewScrap->GetRootComponent()))
				{
					FVector RandomImpulse = FMath::VRand() * 300.f;
					Root->AddImpulse(RandomImpulse + FVector(0,0,500), NAME_None, true);
				}
			}
		}
	}
}
