// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "./ExternalGymConnector.h"
#include "Agent/AgentAction.h"
#include "../Generated/GymConnector.pb.h"
#include "../Generated/StateUpdates.pb.h"
#include "../Generated/State.pb.h"
#include "../Generated/GymConnector.grpc.pb.h"
#include "Communicator/CommunicationManager.h"
#include "Communicator/ExchangeRPCBackend.h"
#include "Communicator/PollingRPCBackend.h"
#include "Communicator/ComBackendInterface.h"
#include "PythonGymConnector.generated.h"


using Schola::InitialTrainingState;
using Schola::InitialTrainingStateRequest;
using Schola::GymService;
using Schola::GymConnectorStartRequest;
using Schola::GymConnectorStartResponse;
using Schola::TrainingDefinition;
using Schola::TrainingDefinitionRequest;
using Schola::TrainingState;
using Schola::TrainingStateUpdate;

/**
 * @brief A struct representing a msg indicating that the connector should start
 */
USTRUCT()
struct FStartRequest
{

	GENERATED_BODY()

	/**
	 * @brief Create a new empty FStartRequest
	 */
	FStartRequest();

	/**
	 * @brief Create a new FStartRequest from a proto msg
	 * @param[out] EmptyResetRequest The FStartRequest to populate
	 * @param[in] ProtoMsg The proto msg to populate from
	 */
	static void FromProto(FStartRequest& EmptyResetRequest, const GymConnectorStartRequest& ProtoMsg);

	/**
	 * @brief Create a new FStartRequest from a proto msg
	 * @param[in] ProtoMsg The proto msg to populate from
	 */
	FStartRequest(const GymConnectorStartRequest& ProtoMsg);

	/**
	 * @brief Create a new FStartRequest from a proto msg
	 * @param[in] ProtoMsg The proto msg to populate from
	 * @return The new FStartRequest
	 */
	static FStartRequest* FromProto(const GymConnectorStartRequest& ProtoMsg);
};

/**
 * @brief A connection to an external gym API that is implemented in python, using gRPC for communication
 * @note This can theoretically work with any gRPC client not just python although that is untested currently.
 */
UCLASS()
class SCHOLA_API UPythonGymConnector : public UExternalGymConnector
{
	GENERATED_BODY()
private:
	/** A type for an exchange interface that Exchanges TrainingState for TrainingStateUpdates */
	typedef IExchangeBackendInterface<TrainingStateUpdate, TrainingState>* DRSType;
	/** A type for a producer interface that sends TrainingDefinition */
	typedef IProducerBackendInterface<TrainingDefinition>*					ADSType;
	/** A type for a producer interface that publishes InitialTrainingStates after each reset */
	typedef IProducerBackendInterface<InitialTrainingState>*				PRSType;
	/** A type for a consumer interface that collects an initial GymConnectorStartRequest */
	IPollingBackendInterface<GymConnectorStartRequest>*						StartRequestService;

	/** The service that will handle decision requests */
	DRSType DecisionRequestService;
	/** The service that will handle publishing agent definitions */
	ADSType AgentDefinitionService;
	/** The service that will handle publishing the state after each reset */
	PRSType PostResetStateService;

	/** The communication manager that will handle the gRPC server */
	UPROPERTY()
	UCommunicationManager* CommunicationManager;
	//Initialized in Constructor for now

public:
	/**
	 * @brief Create a new UPythonGymConnector
	 */
	UPythonGymConnector();

	/**
	 * @brief Request a decision from gym using the current state of the agents from environments
	 * @return A future that will eventually contain decision for all agents in all environments
	 * @note This function is asynchronous and will return immediately
	 */
	TFuture<FTrainingStateUpdate*> RequestStateUpdate() override;
	void						   SendState(const FTrainingState& Value) override;

	void						   SubmitPostResetState(const FTrainingState& Value) override;
	void						   Init(const FSharedTrainingDefinition& AgentDefns) override;
	void						   Enable() override;
	bool						   CheckForStart() override;
};