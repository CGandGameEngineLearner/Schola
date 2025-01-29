// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Training/AbstractTrainer.h"
#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include <Kismet/GameplayStatics.h>
#include "Environment/EnvironmentComponents/AbstractEnvironmentUtilityComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Common/LogSchola.h"
#include "Training/TrainingDefinitionStructs.h"
#include "Training/TrainingStateUpdateStructs.h"
#include "AbstractEnvironment.generated.h"


/**
 * @brief Enum class representing the status of the environment.
 */
UENUM(BlueprintType)
enum class EEnvironmentStatus : uint8
{
	Running	  UMETA(DisplayName = "Running"),
	Resetting UMETA(DisplayName = "Resetting"),
	Completed UMETA(DisplayName = "Completed"),
	Error	  UMETA(DisplayName = "Error")
};

/**
 * An abstract class representing an environment.
 */
UCLASS(Abstract)
class SCHOLA_API AAbstractScholaEnvironment : public AActor
{
	GENERATED_BODY()

protected:
	/** A map from the agent ID to the Agent Object. Filled from AgentControlledPawns when the game begins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<int, AAbstractTrainer*> Trainers = TMap<int, AAbstractTrainer*>();

	/** The current largest Id. Used for Registering New Agents at runtime.*/
	UPROPERTY()
	int MaxId = 0;

	/** The Id of the environment. Set at runtime. */
	UPROPERTY()
	int EnvId;

	/** The status of the environment */
	UPROPERTY()
	EEnvironmentStatus EnvironmentStatus = EEnvironmentStatus::Running;

public:
	/**
	 * @brief Register an individual agent with the environment. Called after the environment is initialized.
	 * @param Agent The agent to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Reinforcement Learning")
	void RegisterAgent(AAbstractTrainer* Agent);

	/**
	 * @brief Register a list of agents with the environment, from a list of pawns with associated agents. Called after the environment is initialized.
	 * @param OutAgentControlledPawnArray An array of pawns in the environment that are controlled by agents
	 */
	virtual void RegisterAgents(TArray<APawn*>& OutAgentControlledPawnArray) PURE_VIRTUAL(UAbstractEnvironment::RegisterAgents, return; );

	/**  A list of utility components that can be used to add additional behaviour such as logging or data collection. */
	UPROPERTY()
	TArray<UAbstractEnvironmentUtilityComponent*> UtilityComponents;

	/**
	 * @brief Collects all the AgentObjects from their pawns and then initializes them. 
	 * @note Must be called after play begins. Calls InitializeEnvironment and RegisterAgents.
	 */
	void Initialize();

	/**
	 * @brief Retrieve all the utility components from the environment. Called after the environment is initialized.
	 */
	void RetrieveUtilityComponents();

	/**
	 * @brief Used on initialization to populate the AgentStateMapping with the AgentStates pointers.
	 * @param[out] OutAgentStatePointers A shared state structure that will be populated with the agent state pointers.
	 */
	void PopulateAgentStatePointers(FSharedEnvironmentState& OutAgentStatePointers);

	/**
	 * @brief Used on initialization to populate the AgentDefinitionMapping with the AgentDefinition pointers.
	 * @param[out] OutEnvDefn A shared environment definition structure that will be populated with the agent definition pointers.
	 */
	void PopulateAgentDefinitionPointers(FSharedEnvironmentDefinition& OutEnvDefn);

	/**
	 * @brief Get the number of agents registered to this environment
	 * @return The Number of Agents registered to this environment
	 */
	int GetNumAgents();

	/**
	 * @brief Convenience function to get the centerpoint of the environment for resetting agents
	 * @return an FVector representation of the center of this environment
	 */
	UFUNCTION(BlueprintCallable, Category = "Reinforcement Learning")
	FVector GetEnvironmentCenterPoint();

	/**
	 * @brief Reset the environment. Note that this does not reset the agent state.
	 * @note Subclasses should implement this method to add logic that runs when the environment is reset
	 */
	virtual void ResetEnvironment() PURE_VIRTUAL(UEnvironmentManager::ResetEnvironment, return; );

	/**
	 * @brief Perform any environment setup like initializing variables, or binding delegates. Occurs before Register Agents.
	 */
	virtual void InitializeEnvironment() PURE_VIRTUAL(UEnvironmentManager::InitializeEnvironment, return; );
	
	/**
	 * @brief Reset the environment and any agents in it. Note: Does not set the state to running.
	 */
	void Reset();

	/**
	 * @brief Set the EnvironmentState to Resetting
	 */
	void MarkCompleted();

	/**
	 * @brief Perform a think step for all agents in the environment. Collects observations and sends them to the agents.
	 */
	void AllAgentsThink();

	/**
	 * @brief Perform an act step for all agents in the environment. Acts on any decisions from the brains
	 * @param[in] EnvUpdate The environment update to act on
	 */
	void AllAgentsAct(const FEnvStep& EnvUpdate);

	/**
	 * @brief Set the Id of this environment. Called when Registering with the subsystem.
	 * @param[in] EnvironmentId The Id of the environment
	 */
	void SetEnvId(int EnvironmentId);

	/**
	 * @brief Set the status of this environment to the given status.
	 * @param[in] NewStatus The new status of the environment
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateStatus(EEnvironmentStatus NewStatus);

	/**
	 * @brief Get the current status of this environment.
	 * @return The status of the environment
	 */
	EEnvironmentStatus GetStatus();

	/**
	* @brief Configure this environment based on arbitrary Options from the GymConnector. Called immediately before the environment is reset, if the gym connector has Options.
	* @param[in] Options A map of options to configure the environment with
	*/
	virtual void SetEnvironmentOptions(const TMap<FString, FString>& Options) PURE_VIRTUAL(UEnvironmentManager::SetEnvironmentOptions, return; );
	
	/**
	 * @brief Configure this environment based on a Seed . Called immediately before the environment is reset, if the gym connector has a new seed supplied.
	 * @param[in] Seed The seed to configure the environment with
	 */
	virtual void SeedEnvironment(int Seed) PURE_VIRTUAL(UEnvironmentManager::SeedEnvironment, return; );
};

/**
 * @brief A blueprintable version of the AbstractScholaEnvironment
 */
UCLASS(Blueprintable)
class SCHOLA_API ABlueprintScholaEnvironment : public AAbstractScholaEnvironment
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void ResetEnvironment();

	UFUNCTION(BlueprintImplementableEvent)
	void RegisterAgents(TArray<APawn*>& OutAgentControlledPawnArray);

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeEnvironment();

	UFUNCTION(BlueprintImplementableEvent)
	void SetEnvironmentOptions(const TMap<FString, FString>& Options);

	UFUNCTION(BlueprintImplementableEvent)
	void SeedEnvironment(int Seed);
};
