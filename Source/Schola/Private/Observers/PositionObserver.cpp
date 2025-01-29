// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Observers/PositionObserver.h"

void UPositionObserver::CollectObservations(FBoxPoint& OutObservations)
{
	AActor* LocalTrackedActor = TrackedActor;
	// We are tracking the owner
	if (!bTrackNonOwner)
	{
		LocalTrackedActor = this->TryGetOwner();
	}

	if (LocalTrackedActor)
	{
		FVector ActorLocation = LocalTrackedActor->GetActorLocation();

		if (bTrackNonOwner && PositionAdjustment == EFrameOfReference::Egocentric)
		{
			// Make it so that the vector is relative to this actor's forward vector

			ActorLocation = this->TryGetOwner()->GetActorTransform().InverseTransformPositionNoScale(ActorLocation);
		}
		else if (bTrackNonOwner && PositionAdjustment == EFrameOfReference::Relative)
		{
			// Make the position relative from the owner
			ActorLocation -= this->TryGetOwner()->GetActorLocation();
		}

		if (bHasXDimensions)
		{
			OutObservations.Values.Add(ActorLocation.X);
		}

		if (bHasYDimensions)
		{
			OutObservations.Values.Add(ActorLocation.Y);
		}

		if (bHasZDimensions)
		{
			OutObservations.Values.Add(ActorLocation.Z);
		}
	}
}

FBoxSpace UPositionObserver::GetObservationSpace() const
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