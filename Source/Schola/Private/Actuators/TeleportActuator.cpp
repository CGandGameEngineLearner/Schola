// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Actuators/TeleportActuator.h"

FDiscreteSpace UTeleportActuator::GetActionSpace()
{
	FDiscreteSpace SpaceDefinition;
	int			   NumDirections = (int32)StaticEnum<ETeleportDirection>()->GetMaxEnumValue();
	
	if (bHasXDimension)
	{
		SpaceDefinition.Add(NumDirections);
	}
	
	if (bHasYDimension)
	{
		SpaceDefinition.Add(NumDirections);
	}

	if (bHasZDimension)
	{
		SpaceDefinition.Add(NumDirections);
	}

	return SpaceDefinition;
}

FVector UTeleportActuator::ConvertActionToFVector(const FDiscretePoint& Action)
{
	FVector OutVector = FVector(0);
	int		Offset = 0;

	if (bHasXDimension)
	{
		OutVector.X = GetVectorDimension(XDimensionSpeed, Action[Offset++]);
	}
	if (bHasYDimension)
	{
		OutVector.Y = GetVectorDimension(YDimensionSpeed, Action[Offset++]);
	}
	if (bHasZDimension)
	{
		OutVector.Z = GetVectorDimension(ZDimensionSpeed, Action[Offset++]);
	}
	return OutVector;
}

float UTeleportActuator::GetVectorDimension(int Speed, int DiscretePointValue)
{

	switch ((ETeleportDirection)DiscretePointValue)
	{
		case ETeleportDirection::Nothing:
			return 0;
		case ETeleportDirection::Forward:
			return Speed;
		case ETeleportDirection::Backward:
			return -1 * Speed;
			break;
		default:
			return 0;
	}
}

void UTeleportActuator::TakeAction(const FDiscretePoint& Action)
{
	AActor* LocalTarget = Target;

	if (Target == nullptr)
	{
		Target = TryGetOwner();
	}

	if (Target != nullptr)
	{
		const FVector& ActionVector = ConvertActionToFVector(Action);
		this->OnTeleportDelegate.Broadcast(ActionVector);
		Target->SetActorLocation(Target->GetActorLocation() + ActionVector, this->bSweep, nullptr, this->TeleportType);
	}
}
