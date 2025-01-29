// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Subsystem/ScholaManagerSubsystem.h"

void UScholaManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GetWorld()->OnWorldBeginPlay.AddUObject(this, &UScholaManagerSubsystem::PrepareSubsystem);
}

void UScholaManagerSubsystem::Deinitialize()
{
}

void UScholaManagerSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Subsystem Tick");

	if (this->GymConnector && this->GymConnector->IsNotStarted())
	{
		bFirstStep = true;
		bool bStarted = this->GymConnector->CheckForStart();

		if (bStarted)
		{
			this->InferenceAgentsThink();
		}
	}

	// Action Phase: We take any actions or Reset the Environment
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agents Acting");
		if (this->GymConnector && this->GymConnector->IsRunning())
		{
			FTrainingStateUpdate* StateUpdate = this->GymConnector->ResolveEnvironmentStateUpdate();
			// Maybe there was nothing to resolve
			if (StateUpdate)
			{
				this->GymConnector->UpdateConnectorStatus(*StateUpdate);
				this->GymConnector->UpdateEnvironments(*StateUpdate);
			}
		}

		// Act for inference agents separately here
		this->InferenceAgentsAct();

		// Reset the environments, if the policy said so
		// Do it after we take inference actions so that if they are linked to the envs they get reset properly
		if (this->GymConnector != nullptr && this->GymConnector->IsRunning())
		{
			this->GymConnector->ResetCompletedEnvironments();
		}
	}

	// Thinking Phase: Send the Last State Update to Gym
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Schola: Agents Thinking");
		if (this->GymConnector && this->GymConnector->IsRunning())
		{
			this->GymConnector->CollectEnvironmentStates();
			this->GymConnector->SubmitEnvironmentStates();
		}
		this->InferenceAgentsThink();
	}

	// self-Reset Phase if we have already run for 1+ steps
	if (this->GymConnector && !bFirstStep && this->GymConnector->IsRunning())
	{
		this->GymConnector->ResetCompletedEnvironments();
	}

	bFirstStep = false;
}

ETickableTickType UScholaManagerSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

//Pipe the UObject GetStatID to the abstract method GetStatId in UTickableWorldSubsystem
TStatId UScholaManagerSubsystem::GetStatId() const
{
	return this->GetStatID();
}

void UScholaManagerSubsystem::RegisterInferenceAgent(UObject* InferenceAgent)
{
	UE_LOG(LogSchola, Log, TEXT("Inference Agent Registered"))
	TScriptInterface<IInferenceAgent>& InterfaceRef = this->InferenceAgents.Emplace_GetRef();
	InterfaceRef.SetObject(InferenceAgent);
	InterfaceRef.SetInterface(Cast<IInferenceAgent>(InferenceAgent));

}

void UScholaManagerSubsystem::PrepareSubsystem()
{

	const UScholaManagerSubsystemSettings* ScholaSettings = GetDefault<UScholaManagerSubsystemSettings>();

	// Don't generate a new gym connector if it doesn't exist
	if (*ScholaSettings->GymConnectorClass != nullptr)
	{
		this->GymConnector = NewObject<UAbstractGymConnector>(this, ScholaSettings->GymConnectorClass, FName("GymConnector"));
		this->GymConnector->Init();
	}

	// Setup the inferencing agents
	CollectInferenceAgents();
	InitializeInferenceAgents();

	// Setup the EnvController
	int NumAgents = 0;
	if (this->GymConnector)
	{
		for (AAbstractScholaEnvironment* Environment : GymConnector->Environments)
		{
			NumAgents += Environment->GetNumAgents();
		}
	};

	if (this->GymConnector && NumAgents > 0)
	{
		this->GymConnector->Enable();

		UE_LOG(LogSchola, Warning, TEXT("Backend Started"));
		// Make the tick start doing it's thing
		bSubsystemPrepared = true;
		bFirstStep = true;

		// Use the config setting, but we can override the config value by passing ScholaDisableScript on the command line
		if (ScholaSettings->bRunScriptOnPlay && !FParse::Param(FCommandLine::Get(), TEXT("ScholaDisableScript")))
		{
			FLaunchableScript TrainingScript = ScholaSettings->GetScript();
			TrainingScript.LaunchScript();
		}
	}
	else if (NumAgents == 0)
	{
		UE_LOG(LogSchola, Log, TEXT("Nothing found to train, skipping script and GymConnector start"))
	}
}

//TODO make this work when the Agent is Stored as a component
void UScholaManagerSubsystem::CollectInferenceAgents()
{
	int Count = 0;
	for (UObject* Object : TObjectRange<UObject>())
	{
		Count++;
		//Object is in the same world as us and is an Agent
		if (Object->GetWorld() == GetWorld() && Object->GetClass()->ImplementsInterface(UInferenceAgent::StaticClass()))
		{
			IInferenceAgent* Agent = Cast<IInferenceAgent>(Object);

			if (Agent->GetBrain() && Agent->GetPolicy() && Agent->GetControlledPawn())
			{
				this->RegisterInferenceAgent(Object);
			}
			else
			{
				UE_LOG(LogSchola, Warning, TEXT("Skipping Registering InferenceAgent %s due to invalid setup"), *Object->GetName());
			}
		}
	}
}

bool UScholaManagerSubsystem::IsTickable() const
{
	return bSubsystemPrepared;
}

void UScholaManagerSubsystem::InferenceAgentsThink()
{
	for (TScriptInterface<IInferenceAgent> Agent : InferenceAgents)
	{
		// Check for agent status
		if (Agent->GetStatus() != EAgentStatus::Error)
		{
			Agent->Think();
		}
		else
		{
			UE_LOG(LogSchola, Warning, TEXT("Agent %s has errored out during think"), *Agent->GetAgentName());
		}
	}
}

void UScholaManagerSubsystem::InferenceAgentsAct()
{
	for (TScriptInterface<IInferenceAgent> Agent : InferenceAgents)
	{
		// Check for agent status
		// If error, log and remove this agent
		if (Agent->GetStatus() != EAgentStatus::Error)
		{
			Agent->Act();
		}
		else
		{
			UE_LOG(LogSchola, Warning, TEXT("Agent %s has errored out during act"), *Agent->GetAgentName());
		}
		
	}
}

void UScholaManagerSubsystem::InitializeInferenceAgents()
{
	for (TScriptInterface<IInferenceAgent> Agent : InferenceAgents)
	{
		Agent->Initialize();
	}
}