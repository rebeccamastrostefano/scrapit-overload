// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "EnemySocketWitch.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API AEnemySocketWitch : public AEnemyBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ProtectionRange = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Shield")
	float RoomSearchRadius = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ShieldRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Shield")
	class UNiagaraSystem* ShieldVfx;

	UPROPERTY()
	AEnemyBase* PrimaryShieldedAlly;

	TArray<AActor*> OverlappedActorsInShieldCheck;

	UPROPERTY()
	TArray<AEnemyBase*> ShieldedAllies;

	UPROPERTY()
	class UNiagaraComponent* ActiveShieldVfx = nullptr;

public:
	AEnemySocketWitch();

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool AttemptStartShield();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateShield();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopShield();

	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetFleeLocation() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetLocationNearAlly() const;

	void PerformShieldOverlap(const FVector& Location, const float Radius);

	UFUNCTION(BlueprintCallable, Category = "AI")
	FORCEINLINE AActor* GetShieldedAlly() const
	{
		return PrimaryShieldedAlly;
	}

	virtual void Attack() override
	{
	} //It's a pacifist;

	virtual void Die() override;
};
