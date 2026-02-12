// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScrapBase.h"
#include "ScrapGeneric.generated.h"

UCLASS()
class SCRAPIT_OVERLOAD_API AScrapGeneric : public AScrapBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScrapGeneric();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	TArray<UStaticMesh*> ScrapMeshes;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	TArray<UMaterialInterface*> Materials;
	
	virtual void OnCollected() override;
};
