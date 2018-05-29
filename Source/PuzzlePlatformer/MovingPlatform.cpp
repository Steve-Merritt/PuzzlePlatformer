// Fill out your copyright notice in the Description page of Project Settings.

#include "MovingPlatform.h"



AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	SetMobility(EComponentMobility::Movable);
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}

	m_globalStartLocation = GetActorLocation();
	m_globalTargetLocation = GetTransform().TransformPosition(TargetLocation);
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		FVector location = GetActorLocation();
		float targetDist = (m_globalTargetLocation - m_globalStartLocation).Size();
		float traveled = (location - m_globalStartLocation).Size();
		if(traveled >= targetDist)
		{
			FVector swap = m_globalStartLocation;
			m_globalStartLocation = m_globalTargetLocation;
			m_globalTargetLocation = swap;
		}

		FVector direction = (m_globalTargetLocation - m_globalStartLocation).GetSafeNormal();
		location += Speed * direction * DeltaTime;
		SetActorLocation(location);
	}
}