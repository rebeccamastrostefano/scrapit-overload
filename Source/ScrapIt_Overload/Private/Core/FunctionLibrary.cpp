// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/FunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "Enemies/EnemyBase.h"

AActor* UFunctionLibrary::GetClosestEnemy(const AActor* Looker, float Radius)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Scope = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Looker->GetOwner());

	//Get all pawns in range
	if (!Looker->GetWorld()->OverlapMultiByChannel(Overlaps, Looker->GetActorLocation(), FQuat::Identity, ECC_Pawn,
	                                               Scope, Params))
	{
		return nullptr;
	}

	AActor* ClosestEnemy = nullptr;
	float MinDistSquared = FMath::Square(Radius);
	const FVector ActorLocation = Looker->GetActorLocation();

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* const OtherActor = Result.GetActor();

		//Check for interface
		if (OtherActor == nullptr || !OtherActor->GetClass()->IsChildOf(AEnemyBase::StaticClass()))
		{
			continue;
		}

		const FVector EnemyLocation = OtherActor->GetActorLocation();

		//Check if in range
		const float DistSquared = FVector::DistSquared(EnemyLocation, ActorLocation);
		if (DistSquared < MinDistSquared)
		{
			MinDistSquared = DistSquared;
			ClosestEnemy = OtherActor;
		}
	}
	return ClosestEnemy;
}
