// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Observers/RotationObserver.h"

void URotationObserver::CollectObservations(FBoxPoint& OutObservations)
{
	AActor* LocalTrackedActor = TrackedActor;
	// We are tracking the owner
	if (!bTrackNonOwner)
	{
		LocalTrackedActor = this->TryGetOwner();
	}

	if (LocalTrackedActor)
	{
		FRotator ActorRotation = LocalTrackedActor->GetActorRotation().GetNormalized();

		if (bHasPitch)
		{
			OutObservations.Values.Add(ActorRotation.Pitch);
		}

		if (bHasYaw)
		{
			OutObservations.Values.Add(ActorRotation.Yaw);
		}

		if (bHasRoll)
		{
			OutObservations.Values.Add(ActorRotation.Roll);
		}
	}
}

FBoxSpace URotationObserver::GetObservationSpace() const
{
	FBoxSpace SpaceDefinition;

	if (bHasPitch)
	{
		SpaceDefinition.Dimensions.Add(PitchBounds);
	}

	if (bHasYaw)
	{
		SpaceDefinition.Dimensions.Add(YawBounds);
	}

	if (bHasRoll)
	{
		SpaceDefinition.Dimensions.Add(RollBounds);
	}

	return SpaceDefinition;
}