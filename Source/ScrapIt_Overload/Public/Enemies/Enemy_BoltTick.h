// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/Damageable.h"
#include "Components/SphereComponent.h"
#include "Interfaces/Enemy.h"
#include "Enemy_BoltTick.generated.h"

class UFloatingPawnMovement;

UENUM()
enum class EState : uint8
{
	ES_Chasing,
	ES_Attacking,
	ES_Cooldown,
	ES_Dead
};

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemy_BoltTick : public APawn, public IDamageable, public IEnemy
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemy_BoltTick();
	virtual void TakeDamage(float DamageAmount) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BoltTickMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HurtboxSphere;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UFloatingPawnMovement* MovementComp;
	
	UPROPERTY(EditAnywhere)
	float BaseHealth;
	
	UPROPERTY(EditAnywhere)
	float Damage = 5.0f;
	
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere)
	int32 ScrapDrop;
	
	UPROPERTY(EditAnywhere)
	float AttackRange = 100.f;
	
	UPROPERTY(EditAnywhere)
	float AttackCooldown = 1.f;
	
	UPROPERTY(VisibleAnywhere)
	float CurrentHealth;
	
	UPROPERTY()
	APawn* MechaTarget;
	
	EState CurrentState = EState::ES_Chasing;
	
	void SpawnScrap();
	
	//Attack
	UFUNCTION()
	void OnHurtboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	FTimerHandle AttackTimer;
	void StartAttack();
	void ExecuteAttack();
	void ResetMovement();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
