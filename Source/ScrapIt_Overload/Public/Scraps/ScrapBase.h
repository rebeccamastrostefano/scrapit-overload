// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Scrappable.h"
#include "ScrapBase.generated.h"

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
	Generic,
	W_Screws,
	W_NailGun,
};

UCLASS()
class SCRAPIT_OVERLOAD_API AScrapBase : public AActor, public IScrappable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScrapBase();

protected:
	EScrapState CurrentState = EScrapState::Idle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ScrapMesh;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	EScrapType ScrapType = EScrapType::Generic;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float BasePullSpeed = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float HoverHeight = 150.0f;
	
	UPROPERTY(EditAnywhere, Category = "Scrap Settings")
	float MaxBoostSpeed = 5.0f;
	
	// Magnet Variables
	UPROPERTY()
	AActor* PullingActor;
	
	float MagnetStrength = 1.0f;
	float MagnetRadius = 1.0f;
	float CollectionDistance = 1.0f;
	
	//How long to wait since last pull from magnet before dropping the scrap
	float MagnetTimeout = 0.5f;
	float LastPullTime = 0.0f;
	FVector TargetHoverLocation = FVector::ZeroVector;

	//Scrappable Interface
	virtual void OnMagnetPulled(AActor* MechaActor, float PullStrength, float PullRadius, float CollectionRadius) override;
	virtual void OnMagnetReleased() override;
	virtual void OnCollected() PURE_VIRTUAL(AScrapBase::OnCollected, );
	
	void RiseUp(const FVector& CurrentLocation, const float DeltaTime);
	void ApplyPullForce(const FVector& CurrentLocation, const float DeltaTime);
	
public:	
	virtual void Tick(float DeltaTime) override;
	
	//SETTERS
	void SetScrapType(EScrapType const Type)
	{
		ScrapType = Type;
	}
	
	//GETTERS
	EScrapType GetScrapType() const
	{
		return ScrapType;
	}
};
