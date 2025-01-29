// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Actuators/AbstractActuators.h"


AActor* UActuator::SpawnActor(TSubclassOf<AActor> Class, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator)
{
	FActorSpawnParameters Parameters = FActorSpawnParameters();
	Parameters.SpawnCollisionHandlingOverride = CollisionHandlingOverride;
	Parameters.TransformScaleMethod = TransformScaleMethod;
	Parameters.Instigator = Instigator;
	Parameters.Owner = Owner;

	return this->GetWorld()->SpawnActor<AActor>(Class, SpawnTransform, Parameters);
};

#if WITH_EDITOR

void UBinaryActuator::SetDebugActions(const TPoint& Temp)
{
	this->DebugBinaryPoint = Temp.Get<FBinaryPoint>().Values;
}

void UBoxActuator::SetDebugActions(const TPoint& Temp)
{
	this->DebugBoxPoint = Temp.Get<FBoxPoint>().Values;
}

void UDiscreteActuator::SetDebugActions(const TPoint& Temp)
{
	this->DebugDiscretePoint = Temp.Get<FDiscretePoint>().Values;
}

#endif