// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Common/InteractionManager.h"

void UInteractionManager::SetupObservers(const TArray<UAbstractObserver*>& InObservers, TArray<UAbstractObserver*>& OutObservers)
{
	for (UAbstractObserver* Observer : InObservers)
	{
		Observer->InitializeObserver();
		OutObservers.Add(Observer);
	}
}

void UInteractionManager::CollectObservationsFromObservers(const TArray<UAbstractObserver*>& InObservers, FDictPoint& OutObservationsMap)
{
	for (UAbstractObserver* Observer : InObservers)
	{
		TPoint& ObservationRef = OutObservationsMap.Add();
		Observer->CollectObservations(ObservationRef);
	}
};

void UInteractionManager::CollectObservationSpaceFromObservers(const TArray<UAbstractObserver*>& InObservers, FDictSpace& OutDictSpace)
{
	int Id = 0;
	for (UAbstractObserver* Observer : InObservers)
	{
		TSpace& Space = OutDictSpace.Add(Observer->GetId(Id++));
		// Make a reference and then pass it in for filling
		Observer->FillObservationSpace(Space);
		UE_LOG(LogSchola, Warning, TEXT("Observation Space filled"));
	}
};

void UInteractionManager::SetupActuators(const TArray<UActuator*>& InActuators, TArray<UActuator*>& OutActuators)
{
	for (UActuator* Actuator : InActuators)
	{
		Actuator->InitializeActuator();
		OutActuators.Add(Actuator);
	}
	
}

void UInteractionManager::SendActionsToActuators(TArray<UActuator*>& OutActuators, const FDictPoint& Actions)
{
	int Id = 0;

	for (UActuator* Actuator : OutActuators)
	{
		Actuator->TakeAction(Actions[Id++]);
	}
};

void UInteractionManager::CollectActionSpaceFromActuators(const TArray<UActuator*>& InActuators, FDictSpace& OutSpaceGroups)
{
	int Id = 0;
	for (UActuator* Actuator : InActuators)
	{
		TSpace& TempGroup = OutSpaceGroups.Add(Actuator->GetId(Id++));

		Actuator->FillActionSpace(TempGroup);
	}
};

void UInteractionManager::Initialize(TArray<UAbstractObserver*>& InObservers, TArray<UActuator*>& InActuators)
{
	// Collect all the attached sensors

	SetupObservers(InObservers, this->Observers);
	CollectObservationSpaceFromObservers(this->Observers, this->InteractionDefn.ObsSpaceDefn);

	this->InteractionDefn.ObsSpaceDefn.InitializeEmptyDictPoint(this->Observations);
	// Collect all the attached Actuators
	SetupActuators(InActuators, this->Actuators);
	CollectActionSpaceFromActuators(this->Actuators, this->InteractionDefn.ActionSpaceDefn);

}

void UInteractionManager::DistributeActions(const FDictPoint& ActionMap)
{
	SendActionsToActuators(this->Actuators, ActionMap);
}

FDictPoint& UInteractionManager::AggregateObservations()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola:Observation Collection");

	// Clear the observations
	Observations.Reset();
	// Collect observaions from the sensors
	CollectObservationsFromObservers(Observers, this->Observations);

	// TODO make this more efficient
	if (this->InteractionDefn.bNormalizeObservations)
	{
		this->InteractionDefn.ObsSpaceDefn.NormalizeObservation(this->Observations);
	}

	return this->Observations;
}
