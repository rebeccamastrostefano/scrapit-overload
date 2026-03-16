// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Objectives/CollectObjective.h"

#include "Kismet/GameplayStatics.h"
#include "Mecha/MechaPawn.h"

void UCollectObjective::ActivateObjective(ARoomManager* RoomManager)
{
	//Spawn the weapon
	const UScrapItGameInstance* GameInstance = GetWorld()->GetGameInstance<UScrapItGameInstance>();
	const UPersistentManager* PersistentManager = GetWorld()->GetGameInstance()->GetSubsystem<UPersistentManager>();

	check(GameInstance != nullptr);
	check(PersistentManager != nullptr);

	if (PersistentManager->GetStartingWeapon() != EScrapType::Generic)
	{
		FVector SpawnLocation = FVector(0.f, 0.f, 150.f);
		const TSubclassOf<AScrapBase> WeaponScrapClass = GameInstance->ScrapTypeToBP[PersistentManager->
			GetStartingWeapon()];
		GetWorld()->SpawnActor<AScrapBase>(WeaponScrapClass, SpawnLocation, FRotator::ZeroRotator);
	}

	AMechaPawn* Player = Cast<AMechaPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (Player != nullptr)
	{
		Player->WeaponSystem->OnWeaponAcquired.AddDynamic(this, &UCollectObjective::OnCollected);
	}
}

void UCollectObjective::OnCollected(EScrapType WeaponScrapType, int32 WeaponLevel)
{
	OnObjectiveFinished.Broadcast();
}
