// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class SCRAPIT_OVERLOAD_API IDamageable
{
	GENERATED_BODY()

public:
	virtual void TakeDamage(const float DamageAmount) = 0;
	virtual void Die() = 0;
};
