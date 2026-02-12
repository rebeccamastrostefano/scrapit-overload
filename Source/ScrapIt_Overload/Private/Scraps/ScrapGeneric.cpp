// Fill out your copyright notice in the Description page of Project Settings.


#include "Scraps/ScrapGeneric.h"
#include "Mecha/MechaPawn.h"
#include "Core/ScrapItGameInstance.h"

// Sets default values
AScrapGeneric::AScrapGeneric()
{
}

// Called when the game starts or when spawned
void AScrapGeneric::BeginPlay()
{
	Super::BeginPlay();
	
	//Randomly choose scrap meshes for basic scrap type
	if (ScrapMeshes.Num() > 0)
	{
		int32 const MeshIndex = FMath::RandRange(0, ScrapMeshes.Num() - 1);
		ScrapMesh->SetStaticMesh(ScrapMeshes[MeshIndex]);
		
	}
	if (Materials.Num() > 0)
	{
		int32 const MaterialIndex = FMath::RandRange(0, Materials.Num() - 1);
		ScrapMesh->SetMaterial(0, Materials[MaterialIndex]);
	}
}

void AScrapGeneric::OnCollected()
{
	//Add to scrap count
	if (AMechaPawn* Mecha = Cast<AMechaPawn>(PullingActor))
	{
		Mecha->AddScrap(1);
	}

	Destroy();
}

