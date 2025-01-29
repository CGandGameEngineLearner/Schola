// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Training/AbstractTrainer.h"
#include "Subsystem/ScholaManagerSubsystem.h"
#include "Agent/AgentComponents/SensorComponent.h"
#include "Observers/AbstractObservers.h"
#include "Subsystem/ScholaManagerSubsystem.h"

const FString AGENT_ACTION_ID = FString("__AGENT__");

AAbstractTrainer::AAbstractTrainer()
{
}

bool AAbstractTrainer::Initialize(int EnvId, int AgentId)
{
	UE_LOG(LogSchola, Log, TEXT("Starting Initialization of Agent %s"), *this->GetName())
	// Check that everything exists
	if (this->GetPawn() == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("No Controlled Pawn."));
		return false;
	}

	UE_LOG(LogSchola, Log, TEXT("Agent is Controlling Pawn %s "), *this->GetPawn()->GetName());

	// Collect all the observers and actuators
	TArray<UActuatorComponent*> ActuatorsTemp;
	this->GetPawn()->GetComponents(ActuatorsTemp);
	for (UActuatorComponent* Actuator : ActuatorsTemp)
	{
		this->Actuators.Add(Actuator->Actuator);
	}
	ActuatorsTemp.Reset();
	this->GetComponents(ActuatorsTemp);
	for (UActuatorComponent* Actuator : ActuatorsTemp)
	{
		this->Actuators.Add(Actuator->Actuator);
	}

	TArray<USensor*> SensorsTemp;
	this->GetPawn()->GetComponents(SensorsTemp);
	for (USensor* Sensor : SensorsTemp)
	{
		this->Observers.Add(Sensor->Observer);
	}
	SensorsTemp.Reset();
	this->GetComponents(SensorsTemp);
	for (USensor* Sensor : SensorsTemp)
	{
		this->Observers.Add(Sensor->Observer);
	}

	// Initialize the Interaction Manager with the Observers and Actuators
	this->InteractionManager->Initialize(this->Observers, this->Actuators);

	// Set the agent state's observation as a pointer to the Interaction Manager's observation
	this->State.Observations = &this->InteractionManager->Observations;

	// Set the ID for the Agent
	UAgentUIDSubsystem* UIDManager = GetWorld()->GetSubsystem<UAgentUIDSubsystem>();
	this->TrainerDefn.UniqueId = UIDManager->GetId();
	this->TrainerDefn.Id.EnvId = EnvId;
	this->TrainerDefn.Id.AgentId = AgentId;
	if (this->bUseCustomName)
	{
		this->TrainerDefn.Name = this->Name;
	}
	else
	{
		this->GetName(this->TrainerDefn.Name);
	}
	
	this->TrainerDefn.PolicyDefinition = &this->InteractionManager->InteractionDefn;

	UE_LOG(LogSchola, Warning, TEXT("Initialization finished"));
	return true;
}

FTrainerState AAbstractTrainer::Think()
{

	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agent Thinking");
	// Always test if we are done.
	State.TrainingStatus = this->ComputeStatus();
	// Set the reward.
	State.Reward = this->ComputeReward();
	//Update the info field
	this->State.Info.Reset();
	this->GetInfo(this->State.Info);
	
	this->InteractionManager->AggregateObservations();

	// Update the message status after the last message is sent
	// so that we don't immediately stop but send one final message with the last state
	if (this->IsDone())
	{
		if (this->GetTrainingMsgStatus() == ETrainingMsgStatus::LastMsgPending)
		{
			this->SetTrainingMsgStatus(ETrainingMsgStatus::LastMsgSent);
		}
		else if (this->GetTrainingMsgStatus() == ETrainingMsgStatus::NoStatus)
		{
			this->SetTrainingMsgStatus(ETrainingMsgStatus::LastMsgPending);
		}
	}

	return State;
}

void AAbstractTrainer::Act(const FAction& Action)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agent Acting");
	this->InteractionManager->DistributeActions(Action.Values);

	this->IncrementStep();
}

void AAbstractTrainer::Reset()
{
	this->ResetTrainer();
	State.Observations->Reset();
	State.Info.Reset();
	this->Step = 0;
	this->InteractionManager->AggregateObservations();
	this->GetInfo(this->State.Info);
	this->SetTrainingStatus(EAgentTrainingStatus::Running);
	this->SetTrainingMsgStatus(ETrainingMsgStatus::NoStatus);
}

void AAbstractTrainer::SetTrainingStatus(EAgentTrainingStatus NewStatus)
{
	this->State.TrainingStatus = NewStatus;
}

void AAbstractTrainer::SetTrainingMsgStatus(ETrainingMsgStatus NewStatus)
{
	this->State.TrainingMsgStatus = NewStatus;
}

EAgentTrainingStatus AAbstractTrainer::GetTrainingStatus()
{
	return this->State.TrainingStatus;
}

ETrainingMsgStatus AAbstractTrainer::GetTrainingMsgStatus()
{
	return this->State.TrainingMsgStatus;
}

bool AAbstractTrainer::IsRunning()
{
	return State.TrainingStatus == EAgentTrainingStatus::Running;
}

bool AAbstractTrainer::IsDone() const
{
	return this->State.IsDone();
}

bool AAbstractTrainer::IsActionStep()
{
	return this->IsDecisionStep() || this->bTakeActionBetweenDecisions;
}

bool AAbstractTrainer::IsDecisionStep(int StepToCheck)
{
	return (StepToCheck % this->DecisionRequestFrequency) == 0;
}

bool AAbstractTrainer::IsDecisionStep()
{
	return this->IsDecisionStep(this->Step);
}