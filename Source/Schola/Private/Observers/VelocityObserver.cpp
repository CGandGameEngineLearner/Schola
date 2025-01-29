// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Observers/VelocityObserver.h"

void UVelocityObserver::CollectObservations(FBoxPoint& OutObservations)
{
	AActor* LocalTrackedActor = TrackedActor;
	// We are tracking the owner
	if (!bTrackNonOwner)
	{
		LocalTrackedActor = this->TryGetOwner();
	}

	if (LocalTrackedActor)
	{
		FVector ActorVelocity = LocalTrackedActor->GetVelocity();

		if (bHasXDimensions)
		{
			OutObservations.Values.Add(ActorVelocity.X);
		}

		if (bHasYDimensions)
		{
			OutObservations.Values.Add(ActorVelocity.Y);
		}

		if (bHasZDimensions)
		{
			OutObservations.Values.Add(ActorVelocity.Z);
		}
	}
}

FBoxSpace UVelocityObserver::GetObservationSpace() const
{
	FBoxSpace SpaceDefinition;

	if (bHasXDimensions)
	{
		SpaceDefinition.Dimensions.Add(XDimensionBounds);
	}

	if (bHasYDimensions)
	{
		SpaceDefinition.Dimensions.Add(YDimensionBounds);
	}

	if (bHasZDimensions)
	{
		SpaceDefinition.Dimensions.Add(ZDimensionBounds);
	}

	return SpaceDefinition;
}