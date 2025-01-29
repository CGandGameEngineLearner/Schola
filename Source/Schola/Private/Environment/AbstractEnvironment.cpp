// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Environment/AbstractEnvironment.h"

// Not used in this file to UtilityComponents Events to initialize before MaxId is incremented.
void AAbstractScholaEnvironment::RegisterAgent(AAbstractTrainer* Agent)
{
	this->Trainers.Add(MaxId, Agent);
	MaxId++;
}

void AAbstractScholaEnvironment::Initialize()
{
	this->InitializeEnvironment();
	TArray<APawn*> AgentControlledPawnsArray;
	RegisterAgents(AgentControlledPawnsArray);
	RetrieveUtilityComponents();

	for (auto& ControlledPawn : AgentControlledPawnsArray)
	{
		if (ControlledPawn == nullptr)
		{
			UE_LOG(LogSchola, Warning, TEXT("Blank Entry in Pawn array for Environment %s. Skipping."), *this->GetName());
		}
		else
		{
			AAbstractTrainer* Agent = Cast<AAbstractTrainer>(ControlledPawn->GetController());
			// Only add the Agent if it successfully initializes.
			if (Agent == nullptr)
			{
				UE_LOG(LogSchola, Warning, TEXT("Pawn %s is Not Controlled by a valid Agent in Environment %s. Skipping."), *ControlledPawn->GetName(), *this->GetName());
			}
			else if (Agent->Initialize(this->EnvId, MaxId))
			{
				Trainers.Add(MaxId, Agent);
				for (UAbstractEnvironmentUtilityComponent* Component : UtilityComponents)
				{
					Component->OnAgentRegister(MaxId);
				}
				// Increment so it is always largets value+1, that way RegisterAgent never clobbers
				MaxId++;
			}
			else
			{
				UE_LOG(LogSchola, Warning, TEXT("Initializing Agent for Pawn %s Failed in Environment %s. Skipping."), *ControlledPawn->GetName(), *this->GetName());
			}
		}
		for (UAbstractEnvironmentUtilityComponent* Component : UtilityComponents)
		{
			Component->OnEnvironmentInit(EnvId);
		}
	}

	if (this->Trainers.Num() <= 0)
	{
		UE_LOG(LogSchola, Warning, TEXT("Environment %s has No Agents. Are you sure this is correct? See previous logs for potential errors while adding agents."), *this->GetName());
	}
}

void AAbstractScholaEnvironment::RetrieveUtilityComponents()
{
	this->GetComponents(UtilityComponents);
}

void AAbstractScholaEnvironment::PopulateAgentStatePointers(FSharedEnvironmentState& OutSharedEnvironmentState)
{
	for (auto& IdAgentPair : this->Trainers)
	{
		OutSharedEnvironmentState.AddSharedAgentState(IdAgentPair.Key, &IdAgentPair.Value->State);
	}
}

void AAbstractScholaEnvironment::PopulateAgentDefinitionPointers(FSharedEnvironmentDefinition& OutEnvDefn)
{
	for (auto& IdAgentPair : this->Trainers)
	{
		// TODO make a method for getting the agent defn
		OutEnvDefn.AddSharedAgentDefn(IdAgentPair.Key, &IdAgentPair.Value->TrainerDefn);
	}
}

int AAbstractScholaEnvironment::GetNumAgents()
{
	return Trainers.Num();
}

FVector AAbstractScholaEnvironment::GetEnvironmentCenterPoint()
{
	return this->GetLevelTransform().GetLocation();
}

void AAbstractScholaEnvironment::Reset()
{
	ResetEnvironment();
	for (auto& IdAgentPair : Trainers)
	{
		AAbstractTrainer* Trainer = IdAgentPair.Value;
		Trainer->Reset();
	}

	for (UAbstractEnvironmentUtilityComponent* Component : UtilityComponents)
	{
		Component->OnEnvironmentReset();
	}
}

void AAbstractScholaEnvironment::MarkCompleted()
{
	this->EnvironmentStatus = EEnvironmentStatus::Completed;
}

void AAbstractScholaEnvironment::AllAgentsThink()
{
	bool AllDone = true;

	for (auto& IdAgentPair : Trainers)
	{
		FTrainerState State = IdAgentPair.Value->Think();

		// Pass agent state to Utility Components for calculations.
		for (UAbstractEnvironmentUtilityComponent* Component : UtilityComponents)
		{
			Component->OnEnvironmentStep(IdAgentPair.Key, State);
		}

		if (!State.IsDone())
		{
			AllDone = false;
		}
	}

	if (AllDone)
	{
		this->EnvironmentStatus = EEnvironmentStatus::Completed;
	}

	// If all agents are done, mark the environment as completed
}

void AAbstractScholaEnvironment::AllAgentsAct(const FEnvStep& EnvUpdate)
{
	for (const TTuple<int, FAction>& IdActionPair : EnvUpdate.Actions)
	{
		// We only act if we are running
		if (Trainers[IdActionPair.Key]->State.TrainingStatus != EAgentTrainingStatus::Running)
		{
			continue;
		}
		Trainers[IdActionPair.Key]->Act(IdActionPair.Value);
	}
}

void AAbstractScholaEnvironment::SetEnvId(int EnvironmentId)
{
	this->EnvId = EnvironmentId;
}

void AAbstractScholaEnvironment::UpdateStatus(EEnvironmentStatus NewStatus)
{
	this->EnvironmentStatus = NewStatus;
}

EEnvironmentStatus AAbstractScholaEnvironment::GetStatus()
{
	return this->EnvironmentStatus;
}
