// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Environment/AbstractEnvironment.h"
#include "Common/LogSchola.h"
#include "CoreMinimal.h"
#include "Communicator/CommunicationManager.h"
#include "Agent/AgentAction.h"
#include "Training/AbstractTrainer.h"
#include "Inference/IInferenceAgent.h"
#include "GymConnectors/AbstractGymConnector.h"
#include <Kismet/GameplayStatics.h>
#include "Subsystem/SubsystemSettings.h"
#include "ScholaManagerSubsystem.generated.h"

/**
 * @brief The core subsystem that coordinates the various parts of the UERL toolkit
 */
UCLASS()
class SCHOLA_API UScholaManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

private:
	/** Boolean variable tracking whether the subsystem has finished initializing, this is separate from the play button on the unreal editor */
	bool bSubsystemPrepared = false;
	
	/** Boolean Variable tracking whether the subsystem has completed it's initial reset */
	bool bFirstStep = true;

protected:
public:
	/** The inferencing agents that are currently being controlled by the subsystem */
	UPROPERTY()
	TArray<TScriptInterface<IInferenceAgent>> InferenceAgents = TArray<TScriptInterface<IInferenceAgent>>();

	/** The gym connector that is currently selected */
	UPROPERTY()
	UAbstractGymConnector* GymConnector;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * @brief Perform an update the currently running environment. Potentially collecting observations on all agents, and doing actions
	 * @param[in] DeltaTime The time since the last tick.
	 */
	virtual void Tick(float DeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId			  GetStatId() const override;

	/**
	 * @brief Register an inferencing agent with the subsystem so that it can be controlled by the subsytem.
	 * @param[in] InferenceAgent The inference agent to be registered.
	 */
	void RegisterInferenceAgent(UObject* InferenceAgent);

	/**
	 * @brief Prepare the subsystem by doing post BeginPlay setup
	 */
	void PrepareSubsystem();

	/**
	 * @brief Collect all the inferencing agents in the simulation
	 */
	void CollectInferenceAgents();

	virtual bool IsTickable() const;

	/**
	 * @brief Take Actions for all the inference agents in the simulation
	 */
	void InferenceAgentsAct();

	/**
	 * @brief Collect Observations and start decision making for all the inference agents in the simulation
	 */
	void InferenceAgentsThink();

	/**
	 * @brief Initialize the inference agents in the simulation
	 */
	void InitializeInferenceAgents();
};
