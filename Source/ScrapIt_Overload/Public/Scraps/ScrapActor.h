// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Scrappable.h"
#include "ScrapActor.generated.h"

UENUM(BlueprintType)
enum class EScrapState : uint8
{
	Idle,
	Rising,
	Pulling
};

UENUM(BlueprintType)
enum class EScrapType : uint8
{
	Basic,
	W_Screws,
	W_NailGun,
};

UCLASS()
class SCRAPIT_OVERLOAD_API AScrapActor : public AActor, public IScrappable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScrapActor();
	virtual void OnMagnetPulled(AActor* MechaActor, float PullStrength, float PullRadius, float CollectionRadius) override;
	virtual void OnMagnetReleased() override;
	virtual void OnCollected() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ScrapMesh;
	
	EScrapState CurrentState = EScrapState::Idle;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	EScrapType ScrapType = EScrapType::Basic;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float BasePullSpeed = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float HoverHeight = 150.0f;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float MaxBoostSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Basic Scrap Settings")
	TArray<UStaticMesh*> BasicScrapMeshes;
	
	UPROPERTY(EditAnywhere, Category = "Basic Scrap Settings")
	TArray<UMaterialInterface*> BasicScrapMaterials;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scrap Settings")
	int32 WeaponLevel = 0;
	
	FVector TargetHoverLocation = FVector::ZeroVector;
	
	UPROPERTY()
	AActor* PullingActor;
	
	float MagnetStrength = 1.0f;
	float MagnetRadius = 1.0f;
	float CollectionDistance = 1.0f;
	
	//How long to wait since last pull from magnet before dropping the scrap
	float MagnetTimeout = 0.5f;
	float LastPullTime = 0.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void InitWeaponScrap(EScrapType WeaponScrapType, const int32 LevelNumber);
	
	//SETTERS
	void SetScrapType(EScrapType const Type)
	{
		ScrapType = Type;
	}
	
	void SetWeaponLevel(const int32 Level)
	{
		WeaponLevel = Level;
	}
	
	//GETTERS
	EScrapType GetScrapType() const
	{
		return ScrapType;
	}
	
	int32 GetWeaponLevel() const
	{
		return WeaponLevel;
	}

};
