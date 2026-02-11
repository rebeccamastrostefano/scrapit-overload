// Fill out your copyright notice in the Description page of Project Settings.


#include "Mecha/TierSystemComponent.h"

// Sets default values for this component's properties
UTierSystemComponent::UTierSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTierSystemComponent::LoadTierState(int32 SavedTierNumber)
{
	CurrentTier = GetTierByNumber(SavedTierNumber);
}

void UTierSystemComponent::InitializeTierSystem()
{
	//Populate Mass Mesh Parts array
	TArray<UStaticMeshComponent*> Meshes; 
	GetOwner()->GetComponents<UStaticMeshComponent>(Meshes);
	
	for (UStaticMeshComponent* Mesh : Meshes)
	{
		if (Mesh->ComponentHasTag("MassTier"))
		{
			const FName* NumericTag = Mesh->ComponentTags.FindByPredicate([](const FName Tag)
			{
				return Tag.ToString().IsNumeric();
			});
			
			if (NumericTag != nullptr)
			{
				int32 TierNumber = FCString::Atoi(*NumericTag->ToString());
				MassMeshesForTiers.Add(Mesh, TierNumber);
			}
		}
	}
	
	CurrentTier = MassTiers[0];
	OnTierChanged.Broadcast(CurrentTier);
}

void UTierSystemComponent::CheckForTierChange(const int32 CurrentScraps)
{
	//Determine if we are upgrading, downgrading or staying same tier
	const int32 TierDirection = (CurrentScraps > CurrentTier.UpgradeThreshold) ? 1 : (CurrentScraps < CurrentTier.DowngradeThreshold) ? -1 : 0;
	
	if (TierDirection == 0)
	{
		return; //No change
	}
	
	//Find the target Tier (previous or next)
	const int32 TargetTierNumber = CurrentTier.TierNumber + TierDirection;
	const FMassTier* NewTier= MassTiers.FindByPredicate([TargetTierNumber](const FMassTier& Tier)
	{
		return Tier.TierNumber == TargetTierNumber;
	});

	if (NewTier != nullptr)
	{
		ApplyNewTier(*NewTier);
	}
}

void UTierSystemComponent::ApplyNewTier(const FMassTier& Tier)
{
	//We are upgrading or downgrading Tier
	CurrentTier = Tier;
	UpdateTierVisuals(CurrentTier);
	
	//Update UI
	OnTierChanged.Broadcast(CurrentTier);
	UE_LOG(LogTemp, Warning, TEXT("Tier Changed to: %d"), Tier.TierNumber);
}

void UTierSystemComponent::UpdateTierVisuals(const FMassTier& Tier)
{
	for (auto& [Mesh, MeshTier] : MassMeshesForTiers)
	{
		if (Mesh != nullptr)
		{
			bool const bIsVisible = MeshTier <= Tier.TierNumber;
			Mesh->SetVisibility(bIsVisible);
		}
	}
}


/* Helper Functions */
FMassTier UTierSystemComponent::GetTierByNumber(const int32 TierNumber) const
{
	const auto FoundTier = MassTiers.FindByPredicate([=](const FMassTier& Tier)
	{
		return Tier.TierNumber == TierNumber;
	});
		
	return FoundTier ? *FoundTier : MassTiers[0];
}
