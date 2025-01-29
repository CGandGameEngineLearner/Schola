// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentAction.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "Policies/PolicyDecision.h"
#include "./IGymConnector.h"
#include "Environment/AbstractEnvironment.h"
#include "AbstractGymConnector.generated.h"

/** Enum class representing the status of the connector. */
UENUM(BlueprintType)
enum class EConnectorStatus : uint8
{
	Running	   UMETA(DisplayName = "Running"),
	Closed	   UMETA(DisplayName = "Closed"),
	Error	   UMETA(DisplayName = "Error"),
	NotStarted UMETA(DisplayName = "NotStarted"),
};

//Maybe we switch to the template pattern instead of delegates, but this lets other objects subscribe to evenets.
DECLARE_MULTICAST_DELEGATE(FConnectorStartedDelegate);
DECLARE_MULTICAST_DELEGATE(FConnectorClosedDelegate);
DECLARE_MULTICAST_DELEGATE(FConnectorErrorDelegate);


/**
 * @brief An abstract class for connectors between unreal and gym.
 */
UCLASS(Blueprintable, Abstract)
class SCHOLA_API UAbstractGymConnector : public UObject, public IGymConnector
{
	GENERATED_BODY()

public:
	/** The current state update from the environments */
	FTrainingStateUpdate* CurrentStateUpdate;
	
	/** Delegate for when the connector starts */
	FConnectorStartedDelegate OnConnectorStarted;

	/** Delegate for when the connector closes */
	FConnectorClosedDelegate  OnConnectorClosed;

	/** Delegate for when the connector encounters an error */
	FConnectorErrorDelegate	  OnConnectorError;

	/** The status of the connector */
	UPROPERTY()
	EConnectorStatus Status = EConnectorStatus::Running;

	/** The environments that are currently being trained */
	UPROPERTY()
	TArray<AAbstractScholaEnvironment*> Environments = TArray<AAbstractScholaEnvironment*>();

	/** The states of the environments that are currently being trained */
	UPROPERTY()
	FTrainingState SharedTrainingState = FTrainingState();

	/** The shared training definition for the environments that will be trained*/
	UPROPERTY()
	FSharedTrainingDefinition SharedTrainingDefinition = FSharedTrainingDefinition();

	/**
	 * @brief Constructor for the abstract gym connector
	 */
	UAbstractGymConnector();
	
	/**
	 * @brief Initialize this gym connector, setting up services and sending agent definitions
	 * @param AgentDefinitions - The definitions of the agents that will be trained
	 * @note This function should be implemented by a derived class
	 */
	virtual void				  Init(const FSharedTrainingDefinition& AgentDefinitions);


	/**
	 * @brief Initialize this gym connector, collecting environments and training definitions
	 * @note Calls the other Init function with the SharedTrainingDefinition
	 */
	void						  Init();

	/**
	 * @brief Reset all the environments that have completed
	 */
	virtual void				  ResetCompletedEnvironments() override;

	/**
	 * @brief Update the environments with the new state update
	 * @param[in] StateUpdate The new state update
	 */
	virtual void				  UpdateEnvironments(FTrainingStateUpdate& StateUpdate) override;

	/**
	 * @brief Collect all the environment states
	 */
	virtual void				  CollectEnvironmentStates();

	/**
	 * @brief Set the status of the connector
	 * @param[in] NewStatus The new status of the connector
	 */
	void						  SetStatus(EConnectorStatus NewStatus);

	/**
	 * @brief Submit environment states to the other end of the connector
	 * @note This function should be implemented by a derived class
	 */
	virtual void				  SubmitEnvironmentStates() PURE_VIRTUAL(UAbstractGymConnector::SubmitEnvironmentStates, return; );

	/**
	 * @brief Resolve the environment state update. Useful for connections that operate off of futures, or otherwise require synchronization.
	 * @return The resolved environment state update
	 * @note This function should be implemented by a derived class
	 */
	virtual FTrainingStateUpdate* ResolveEnvironmentStateUpdate() PURE_VIRTUAL(UAbstractGymConnector::ResolveEnvironmentStateUpdate, return nullptr;);

	/**
	 * @brief Submit the initial state of the environment after a reset to the other end of the connector
	 * @param[in] States The states to submit
	 * @note This function should be implemented by a derived class
	 */
	virtual void				  SubmitPostResetState(const FTrainingState& States) PURE_VIRTUAL(UAbstractGymConnector::SubmitPostResetState, return; );

	/**
	 * @brief Update the connector status based on a state update
	 * @param[in] StateUpdate The state update to base the new status on
	 */
	void						  UpdateConnectorStatus(const FTrainingStateUpdate& StateUpdate);

	/**
	 * @brief Update the connector status based on the last state update
	 */
	void						  UpdateConnectorStatus();

	/**
	 * @brief Enable the connector. Useful for multistage setup as it is called after init.
	 * @note This function should be implemented by a derived class
	 */
	virtual void				  Enable() PURE_VIRTUAL(UAbstractGymConnector::Enable, return; );

	/**
	 * @brief Check if the connector is ready to start
	 * @return True if the connector is ready to start
	 * @note This function should be implemented by a derived class
	 */
	virtual bool				  CheckForStart() PURE_VIRTUAL(UAbstractGymConnector::CheckForStart, return true;);

	/**
	 * @brief Get the latest state update
	 * @return The last state update
	 */
	virtual FTrainingStateUpdate* GetCurrentStateUpdate() { return this->CurrentStateUpdate; };

	/**
	 * @brief Get if the connector is running
	 * @return True iff the connector is running
	 */
	bool IsRunning() { return Status == EConnectorStatus::Running; };

	/**
	 * @brief Get if the connector is closed
	 * @return True iff the connector is closed
	 */
	bool IsNotStarted() { return Status == EConnectorStatus::NotStarted || Status == EConnectorStatus::Closed; };
	
	/**
	 * @brief Collect all the EnvironmentManagers in the simulation
	 */
	void CollectEnvironments() override;

	/**
	 * @brief Register an environment with the subsystem so that it can be controlled by the subsytem.
	 * @param[in] Env A ptr to the environment to be registered.
	 */
	void RegisterEnvironment(AAbstractScholaEnvironment* Env);
};