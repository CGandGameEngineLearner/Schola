// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/SortedMap.h"
#include "AIController.h"
#include "Common/InteractionDefinition.h"
#include "Training/TrainingStateStructs.h"
#include "Training/TrainingDefinitionStructs.h"
#include "Agent/AgentAction.h"
#include "Agent/AgentComponents/SensorComponent.h"
#include "Observers/AbstractObservers.h"
#include "Components/ActorComponent.h"
#include "Agent/AgentUIDSubsystem.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "Common/LogSchola.h"
#include "Common/IValidatable.h"
#include "Common/InteractionManager.h"
#include "Actuators/AbstractActuators.h"
#include "Agent/AgentComponents/ActuatorComponent.h"
#include "AbstractTrainer.generated.h"

/**
 * @brief An abstract class representing a controller that trains an NPC using Reinforcement Learning.
 * @note This class is designed to be subclassed in C++ or Blueprint to implement the specific training logic for an NPC.
 * @note This class is designed to be used in conjunction with the AbstractEnvironment class.
 */
UCLASS(Abstract)
class SCHOLA_API AAbstractTrainer : public AController
{
	GENERATED_BODY()

public:
	/**
	 * @brief Construct a new AAbstractTrainer object
	 */
	AAbstractTrainer();

	/** The current state of the agent*/
	UPROPERTY()
	FTrainerState State = FTrainerState();

	/** An Object for managing the Interactions of the agent and the environment */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	UInteractionManager* InteractionManager = CreateDefaultSubobject<UInteractionManager>(TEXT("InteractionManager"));

	/** List of observers that collect observations for the agent. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	TArray<UAbstractObserver*> Observers;

	/** List of actuators that execute actions for the agent. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	TArray<UActuator*> Actuators;

	/** The type of validation to perform on this agent. Fail means agent is skipped on any error. Warning means just warn about non-fatal errors. Set to No Validation to skip validation */
	UPROPERTY(EditAnywhere, Category = "Reinforcement Learning")
	EValidationType Validation = EValidationType::FAIL;

	/** If true the agent will repeat its last action each step between decision requests */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAbstractSettingsVisibility", EditConditionHides, HideEditConditionToggle), Category = "Reinforcement Learning")
	bool bTakeActionBetweenDecisions = true;

	
	/** The current step of the agent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reinforcement Learning")
	int Step = 0;
	
	/** The name of the agent, used for logging, and grouping agents in rllib */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (InlineEditConditionToggle), Category = "Reinforcement Learning")
	bool bUseCustomName = false;

	/** The name of the agent, used for logging, and grouping agents in rllib */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseCustomName"), Category = "Reinforcement Learning")
	FString Name;

	/** The number of steps between requests for new actions. If this is different across agents it may cause issues training in some frameworks (e.g. Stable Baselines 3). */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAbstractSettingsVisibility", EditConditionHides, HideEditConditionToggle), Category = "Reinforcement Learning")
	int DecisionRequestFrequency = 5;

	/** Whether settings in this class are visible in the editor for child classes. Set to False to hide those settings. */
	UPROPERTY()
	bool bAbstractSettingsVisibility = true;

	/** The definition of the agent */
	UPROPERTY()
	FTrainerDefinition TrainerDefn;

	/**
	 * @brief Initialize this agent after play has begun.
	 * @param[in] EnvId The ID of the environment this agent is in.
	 * @param[in] AgentId The ID of this agent in the environment.
	 */
	bool Initialize(int EnvId, int AgentId);

	/**
	 * @brief Collect a reward from the agent's immediate environment.
	 * @return float representing the agents reward
	 * @note This function must be implemented by a subclass.
	 */
	virtual float ComputeReward() PURE_VIRTUAL(UAbstractAgent::ComputeReward, return 0.0;);
	/**
	 * @brief Check if agent is in a terminal state.
	 * @return The status of the agent which informs whether it is still running, or why it stopped.
	 * @note This function must be implemented by a subclass.
	 */
	virtual EAgentTrainingStatus ComputeStatus() PURE_VIRTUAL(UAbstractAgent::ComputeStatus, return EAgentTrainingStatus::Running; );

	/**
	 * @brief Get the Info output of the agent.
	 * @param[out] Info A mapping representing non-observation details about the environment.
	 */
	virtual void GetInfo(TMap<FString, FString>& Info) PURE_VIRTUAL(UAbstractAgent::GetInfo, return; );

	/**
	 * @brief Get the last computed training status.
	 * @return The last computed training status.
	 */
	EAgentTrainingStatus GetTrainingStatus();

	/**
	 * @brief Get whether the agent has finished, and if it has sent a final state update noting that it is finished.
	 * @return An enum tracking whether the environment has sent a final state update after ending.
	 */
	ETrainingMsgStatus GetTrainingMsgStatus();

	/**
	 * @brief Set the Agent's Training Status
	 * @param[in] NewStatus The new status to set
	 */
	void SetTrainingStatus(EAgentTrainingStatus NewStatus);

	/**
	 * @brief Set the Agent's Training Message Status
	 * @param[in] NewStatus The new status to set
	 */
	void SetTrainingMsgStatus(ETrainingMsgStatus NewStatus);

	/**
	 * @brief Does this agent need resetting (either Truncated or Complete)
	 * @return true iff the agent needs resetting
	 */
	UFUNCTION()
	bool IsDone() const;

	/**
	 * @brief Reset the agent. Collect initial observations of the environment after resetting in the process.
	 */
	UFUNCTION()
	void Reset();
	
	/**
	* @brief Reset any per Episode properties of this Trainer.
	* @note This function must be implemented by a subclass.
	*/
	virtual void ResetTrainer() PURE_VIRTUAL(AAbstractTrainer::ResetTrainer, return;);

	/**
	 * @brief increment the step count for this episode. This is used to determine when to request a new action from the brain.
	 * 
	 */
	void IncrementStep() { Step++; }

	/** 
	 * @brief Reset the step count to 0. This is used to determine when to request a new action from the brain.
	*/
	void ResetStep() { Step = 0; }

	/**
	 * @brief Check whether a specific step will require a brain decision
	 * @param StepToCheck the timestep to check
	 * @return true iff the agent should be requesting a decision
	 */
	virtual bool IsDecisionStep(int StepToCheck);

	/**
	 * @brief If the current step is a decision step, as defined by the step frequency
	 * @return true iff the current step is a decision step
	 */
	virtual bool IsDecisionStep();

	/**
	 * @brief Check if brain has an action, and it's an action step
	 * @return true iff the agent should take an action this step
	 */
	virtual bool IsActionStep();

	/**
	 * @brief The Agent retrieves an action from the brain before taking it
	 * @param[in] Action The action to take
	 */
	void Act(const FAction& Action);

	/**
	 * @brief Update the state of the agent. This checks if the agent is done, what it's reward should be and does any observation collection before requesting a decision
	 * @return The state of the agent after the update
	 */
	FTrainerState Think();

	/**
	 * @brief Check with brain if can act and set agent state accordingly
	 * @return The state of the agent after the update
	 */
	bool IsRunning();

};

/**
 * @brief A blueprint subclass of AAbstractTrainer that implements the abstract methods.
 * @note This class is designed to be subclassed in Blueprint to implement the specific training logic for an NPC.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Schola), meta = (BlueprintSpawnableComponent))
class SCHOLA_API ABlueprintTrainer : public AAbstractTrainer
{
	GENERATED_BODY()

public:

	/**
	 * @brief Collect a reward from the agent's immediate environment. Implemented by a blueprint subclass
	 * @return float representing the agents reward
	 */
	UFUNCTION(BlueprintImplementableEvent)
	float ComputeReward() override;

	/**
	 * @brief Check if agent is in a terminal state. Implemented by a blueprint subclass
	 * @return The status of the agent which informs whether it is still running, or why it stopped.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	EAgentTrainingStatus ComputeStatus() override;

	/**
	* @brief Get the Info Mapping for the Agent. Implemented by a blueprint subclass
	* @param[out] Info A map containing additional info collected by the agent during training
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void GetInfo(TMap<FString, FString>& Info) override;

	/**
	 * @brief Reset any per Episode properties of this Trainer. Implemented by a blueprint subclass
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ResetTrainer() override;
};