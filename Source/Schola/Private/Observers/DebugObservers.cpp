// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Observers/DebugObservers.h"

FBoxSpace UDebugBoxObserver::GetObservationSpace() const
{
	return this->ObservationSpace;
}

void UDebugBoxObserver::CollectObservations(FBoxPoint& OutObservations)
{
	for (const FBoxSpaceDimension& BoxSpaceDim : this->ObservationSpace.Dimensions)
	{
		OutObservations.Add(FMath::FRandRange(BoxSpaceDim.Low,BoxSpaceDim.High));
	}
}

FBinarySpace UDebugBinaryObserver::GetObservationSpace() const
{
	return this->ObservationSpace;
}

void UDebugBinaryObserver::CollectObservations(FBinaryPoint& OutObservations)
{
	for (int i = 0; i < this->ObservationSpace.Shape; i++)
	{
		OutObservations.Add(FMath::RandBool());
	}
}

FDiscreteSpace UDebugDiscreteObserver::GetObservationSpace() const
{
	return this->ObservationSpace;
}

void UDebugDiscreteObserver::CollectObservations(FDiscretePoint& OutObservations)
{
	for (auto& DimUpperBound : this->ObservationSpace.High)
	{
		OutObservations.Add(FMath::RandHelper(DimUpperBound));
	}
}
