// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Actuators/MovementInputActuator.h"

FBoxSpace UMovementInputActuator::GetActionSpace()
{
	FBoxSpace OutSpace;
	if (bHasXDimension)
	{
		OutSpace.Add(MinSpeed, MaxSpeed);
	}

	if (bHasYDimension)
	{
		OutSpace.Add(MinSpeed, MaxSpeed);
	}

	if (bHasZDimension)
	{
		OutSpace.Add(MinSpeed, MaxSpeed);
	}
	return OutSpace;
}

FVector UMovementInputActuator::ConvertActionToFVector(const FBoxPoint& Action)
{
	FVector OutVector;
	int		Offset = 0;
	if (bHasXDimension)
	{
		OutVector.X = Action[Offset];
		Offset++;
	}

	if (bHasYDimension)
	{
		OutVector.Y = Action[Offset];
		Offset++;
	}

	if (bHasZDimension)
	{
		OutVector.Z = Action[Offset];
		Offset++;
	}
	
	if(this->bClipMovementInputs)
	{
		if (bHasXDimension)
		{
			OutVector.X = FMath::Clamp(OutVector.X, MinSpeed, MaxSpeed);
		}

		if (bHasYDimension)
		{
			OutVector.Y = FMath::Clamp(OutVector.Y, MinSpeed, MaxSpeed);
		}

		if (bHasZDimension)
		{
			OutVector.Z = FMath::Clamp(OutVector.Z, MinSpeed, MaxSpeed);
		}
	}
	return OutVector;
}

void UMovementInputActuator::TakeAction(const FBoxPoint& Action)
{
	int Offset = 0;

	APawn* LocalTarget = Target;

	if (Target == nullptr)
	{
		Target = Cast<APawn>(TryGetOwner());
	}

	if (Target != nullptr)
	{
		const FVector& ActionVector = ConvertActionToFVector(Action);
		
		this->OnMovementDelegate.Broadcast(ActionVector);
		Target->AddMovementInput(Target->GetActorRotation().RotateVector(ActionVector), ScaleValue, bForce);
	}
}
