// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Inference/IInferenceAgent.h"

TArray<UAbstractObserver*> IInferenceAgent::GetObserversFromPawn()
{
	TArray<UAbstractObserver*> OutObservers;
	TArray<USensor*>		   Sensors;
	this->GetControlledPawn()->GetComponents(Sensors);
	for (USensor* Sensor : Sensors)
	{
		OutObservers.Add(Sensor->Observer);
	}
	return OutObservers;
}

TArray<UActuator*> IInferenceAgent::GetActuatorsFromPawn()
{
	TArray<UActuator*>			OutActuators;
	TArray<UActuatorComponent*> ActuatorsTemp;
	this->GetControlledPawn()->GetComponents(ActuatorsTemp);
	for (UActuatorComponent* Actuator : ActuatorsTemp)
	{
		OutActuators.Add(Actuator->Actuator);
	}
	return OutActuators;
}

FString IInferenceAgent::GetAgentName()
{
	return this->GetControlledPawn()->GetName();
}

bool IInferenceAgent::Initialize()
{
	// UE_LOG(LogSchola, Log, TEXT("Starting Initialization of Agent %s"), *this->GetName())
	// Check that everything exists
	if (this->GetControlledPawn() == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("No Controlled Pawn."));
		return false;
	}

	UE_LOG(LogSchola, Log, TEXT("Agent is Controlling Pawn %s "), *this->GetControlledPawn()->GetName());

	if (this->GetPolicy() == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("No Policy Detected."));
		return false;
	}

	if (this->GetBrain() == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("No Brain Detected."));
		return false;
	}
	TArray<UAbstractObserver*> Observers = this->GetAllObservers();
	TArray<UActuator*>		   Actuators = this->GetAllActuators();

	GetInteractionManager()->Initialize(Observers, Actuators);

	// Setup the Policy
	this->GetPolicy()->Init(this->GetInteractionManager()->InteractionDefn);
	// Configure the brain to use the policy
	this->GetBrain()->Init(this->GetPolicy());
	UE_LOG(LogSchola, Warning, TEXT("Initialization finished"));
	return true;
}

void IInferenceAgent::Think()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agent Thinking");

	// Request on DecisionStep; always request a decision if we are completed, skip if the policy is closed
	if (this->GetBrain()->IsDecisionStep() && this->GetStatus() == EAgentStatus::Running)
	{
		// Clear and then get the observations
		FDictPoint& Obs = GetInteractionManager()->AggregateObservations();

		bool bRequestSuceeded = this->GetBrain()->RequestDecision(Obs);

		// If the status failed error out, which currently should not happen if using synchronous brain
		if (!bRequestSuceeded)
		{
			this->SetStatus(EAgentStatus::Error);
			UE_LOG(LogSchola, Warning, TEXT("Error Durring Agent Step"));
		}
	}
	// No need to perform skip, because not connected to external that requires it
}

void IInferenceAgent::Act()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agent Acting");

	// We only get actions if we are in the action step
	if (this->GetBrain()->IsActionStep() && this->GetStatus() == EAgentStatus::Running)
	{
		// Resolve the decision, in which the brain status should be updated
		this->GetBrain()->ResolveDecision();

		// Check brain status
		// If we have an action ready, we can act
		if (this->GetBrain()->GetStatus() == EBrainStatus::ActionReady)
		{
			FDictPoint& ActionMap = this->GetBrain()->GetAction()->Values;
			GetInteractionManager()->DistributeActions(ActionMap);
		}
		// If we are erroring out, we set the agent status, which will be checked by the subsystem
		else if (this->GetBrain()->GetStatus() == EBrainStatus::Error)
		{
			this->SetStatus(EAgentStatus::Error);
		}
	}
	this->GetBrain()->IncrementStep();
}
