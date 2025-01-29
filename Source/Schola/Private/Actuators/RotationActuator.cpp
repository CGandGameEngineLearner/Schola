// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Actuators/RotationActuator.h"


FBoxSpace URotationActuator::GetActionSpace()
{
	FBoxSpace SpaceDefinition;

	if (bHasPitch)
	{
		if (bNormalizeAndRescale)
		{
			SpaceDefinition.Dimensions.Add(FBoxSpaceDimension::ZeroOneUnitDimension());
		}
		else
		{
			SpaceDefinition.Dimensions.Add(PitchBounds);
		}
		
	}

	if (bHasYaw)
	{
		if (bNormalizeAndRescale)
		{
			SpaceDefinition.Dimensions.Add(FBoxSpaceDimension::ZeroOneUnitDimension());
		}
		else
		{
			SpaceDefinition.Dimensions.Add(YawBounds);
		}
		
	}

	if (bHasRoll)
	{
		if (bNormalizeAndRescale)
		{
			SpaceDefinition.Dimensions.Add(FBoxSpaceDimension::ZeroOneUnitDimension());
		}
		else
		{
			SpaceDefinition.Dimensions.Add(RollBounds);
		}
	}

	return SpaceDefinition;
}

FRotator URotationActuator::ConvertActionToFRotator(const FBoxPoint& Action)
{
	float Pitch = 0;
	float Yaw = 0;
	float Roll = 0;

	int Offset = 0;
	if (bHasPitch)
	{
		Pitch = Action[Offset++];
		if (bNormalizeAndRescale)
		{
			Pitch = PitchBounds.RescaleValue(Pitch);
		}
	}

	if (bHasRoll)
	{	
		Roll = Action[Offset++];
		if (bNormalizeAndRescale)
		{
			Roll = RollBounds.RescaleValue(Roll);
		}
	}

	if (bHasYaw)
	{
		Yaw = Action[Offset++];
		if (bNormalizeAndRescale)
		{
			Yaw = YawBounds.RescaleValue(Yaw);
		}
	}

	return FRotator(Pitch, Yaw, Roll);
}

void URotationActuator::TakeAction(const FBoxPoint& Action)
{

	int		Offset = 0;
	APawn*  LocalTarget = Target;

	if (Target == nullptr)
	{
		Target = Cast<APawn>(TryGetOwner());
	}

	if (Target != nullptr)
	{
		const FRotator& Rotation = ConvertActionToFRotator(Action);
		this->OnRotationDelegate.Broadcast(Rotation);
		Target->AddActorLocalRotation(Rotation, bSweep, nullptr, TeleportType);
	}
}