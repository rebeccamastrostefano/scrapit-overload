// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomObjective.generated.h"

class ARoomManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveFinished);

UCLASS()
class SCRAPIT_OVERLOAD_API URoomObjective : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	URoomObjective();

	UPROPERTY(BlueprintAssignable)
	FOnObjectiveFinished OnObjectiveFinished;

	UFUNCTION()
	virtual void ActivateObjective(ARoomManager* RoomManager) PURE_VIRTUAL(ARoomObjective::ActivateObjective);
};
