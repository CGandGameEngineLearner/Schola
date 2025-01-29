// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Brains/AbstractBrain.h"
#include "Common/InteractionManager.h"
#include "IInferenceAgent.generated.h"

/**
 * @brief The Agent State as represented in UERL, In this case Stopped means the agent is not taking new instructions.
 */
UENUM(BlueprintType)
enum class EAgentStatus : uint8
{
	Running UMETA(DisplayName = "Running"),
	Stopped UMETA(DisplayName = "Stopped"),
	Error	UMETA(DisplayName = "Error")
};

/**
 * @brief An interface implemented by classes that represent an inference agent.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UInferenceAgent : public UInterface
{
	GENERATED_BODY()
};

class IInferenceAgent
{
	GENERATED_BODY()

public:
	/**
	 * @brief Get the controlled pawn of the agent.
	 * @return A ptr to a controlled pawn object.
	 */
	virtual APawn* GetControlledPawn() PURE_VIRTUAL(IInferenceAgent::GetControlledPawn, return nullptr;);

	/**
	 * @brief Get the interaction manager for collecting actuators and observations.
	 * @return A ptr to an interaction manager object.
	 */
	virtual UInteractionManager* GetInteractionManager() PURE_VIRTUAL(IInferenceAgent::GetInteractionManager, return nullptr;);

	/**
	 * @brief Get the brain of the agent.
	 * @return A ptr to a brain object.
	 */
	virtual UAbstractBrain* GetBrain() PURE_VIRTUAL(IInferenceAgent::GetBrain, return nullptr;);
	/**
	 * @brief Get the policy of the agent.
	 * @return a ptr to a policy object.
	 */
	virtual UAbstractPolicy* GetPolicy() PURE_VIRTUAL(IInferenceAgent::GetPolicy, return nullptr;);

	/**
	 * @brief Get both the observers attached to controlled pawn and the observers attached to the agent.
	 * @return An array of observer objects.
	 */
	virtual TArray<UAbstractObserver*> GetAllObservers() PURE_VIRTUAL(IInferenceAgent::RetrieveAllObservers, return TArray<UAbstractObserver*>(););

	/**
	 * @brief Get both the actuators attached to controlled pawn and the actuators attached to the agent
	 * @return An array of actuator objects.
	 */
	virtual TArray<UActuator*> GetAllActuators() PURE_VIRTUAL(IInferenceAgent::RetrieveAllActuators, return TArray<UActuator*>(););

	/**
	 * @brief Get the status of the agent.
	 * @return The status of the agent.
	 */
	virtual EAgentStatus GetStatus() PURE_VIRTUAL(IInferenceAgent::GetStatus, return EAgentStatus::Running;);

	/**
	 * @brief Set the status of the agent.
	 * @param NewStatus The new status to set.
	 */
	virtual void		 SetStatus(EAgentStatus NewStatus) PURE_VIRTUAL(IInferenceAgent::SetStatus, return;);

	/**
	 * @brief Get All observers attached to a pawn
	 * @return An array of observer objects.
	 */
	TArray<UAbstractObserver*> GetObserversFromPawn();

	/**
	 * @brief Get All actuators attached to a pawn
	 * @return An array of actuator objects.
	 */
	TArray<UActuator*>		   GetActuatorsFromPawn();

	/**
	 * @brief Get the name of the agent.
	 * @return The name of the agent.
	 */
	FString					   GetAgentName();

	/**
	 * @brief Initialize this agent after play has begun.
	 * @return True if initialization was successful, false otherwise.
	 */
	bool Initialize();

	/**
	 * @brief The Agent retrieves an action from the brain before taking an action
	 */
	void Act();

	/**
	 * @brief Update the state of the agent. This checks if the agent is done, what it's reward should be and does any observation collection before requesting a decision
	 */
	void Think();
};