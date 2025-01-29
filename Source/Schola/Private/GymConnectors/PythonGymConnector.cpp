// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "GymConnectors/PythonGymConnector.h"

UPythonGymConnector::UPythonGymConnector() {
	this->CommunicationManager = NewObject<UCommunicationManager>();
}

TFuture<FTrainingStateUpdate*> UPythonGymConnector::RequestStateUpdate()
{
	return DecisionRequestService->ReceiveAndDeserialize<FTrainingStateUpdate>();
}

void UPythonGymConnector::SendState(const FTrainingState& State)
{
	Schola::TrainingState* TrainingStateMsg = State.ToProto();
	DecisionRequestService->Respond(TrainingStateMsg);
}

void UPythonGymConnector::SubmitPostResetState(const FTrainingState& States)
{
	Schola::InitialTrainingState* PostResetMessage = new Schola::InitialTrainingState();

	TArray<int> EnvsToReset;
	for (int i = 0; i < Environments.Num(); i++)
	{
		if (Environments[i]->GetStatus() == EEnvironmentStatus::Completed)
		{
			EnvsToReset.Add(i);
		}
	}

	UE_LOG(LogSchola, Verbose, TEXT("Sending Messages for %d Environments"), EnvsToReset.Num());

	PostResetStateService->SendProtobufMessage(States.ToResetProto(EnvsToReset));
}

void UPythonGymConnector::Init(const FSharedTrainingDefinition& AgentDefns)
{
	std::shared_ptr<GymService::AsyncService> Service = std::make_shared<GymService::AsyncService>();
	const UScholaManagerSubsystemSettings*	  Settings = GetDefault<UScholaManagerSubsystemSettings>();
	this->Timeout = Settings->CommunicatorSettings.Timeout;

	this->CommunicationManager->Initialize();

	DecisionRequestService = this->CommunicationManager->CreateExchangeBackend<GymService::AsyncService, TrainingStateUpdate, TrainingState>(&GymService::AsyncService::RequestUpdateState, Service);

	// Watch out for clobbering between Schola::AgentDefinition and the parameter

	PostResetStateService = this->CommunicationManager->CreateProducerBackend<GymService::AsyncService, InitialTrainingStateRequest, InitialTrainingState>(&GymService::AsyncService::RequestRequestInitialTrainingState, Service);

	AgentDefinitionService = CommunicationManager->CreateProducerBackend<GymService::AsyncService, TrainingDefinitionRequest, TrainingDefinition>(&GymService::AsyncService::RequestRequestTrainingDefinition, Service);

	StartRequestService = this->CommunicationManager->CreatePollingBackend<GymService::AsyncService, GymConnectorStartRequest, GymConnectorStartResponse>(&GymService::AsyncService::RequestStartGymConnector, Service);

	// Call initialize
	this->AgentDefinitionService->Initialize();
	this->DecisionRequestService->Initialize();
	this->PostResetStateService->Initialize();
	this->StartRequestService->Initialize();

	this->OnConnectorStarted.AddLambda(
		[this]() {
			this->AgentDefinitionService->SendProtobufMessage(this->SharedTrainingDefinition.ToProtobuf());
		});

	//Since we will have a msg with no response (the closed message, send one more to reset everything)
	this->OnConnectorClosed.AddLambda([this]() {
		this->DecisionRequestService->Respond(new Schola::TrainingState());
	});

}

void UPythonGymConnector::Enable()
{
	this->CommunicationManager->StartBackends();
}

bool UPythonGymConnector::CheckForStart()
{
	TOptional<FStartRequest*> ResetRequest = this->StartRequestService->PollAndDeserialize<FStartRequest>();
	if (ResetRequest.IsSet() || this->Status == EConnectorStatus::Running)
	{
		this->SetStatus(EConnectorStatus::Running);
	}
	return this->Status == EConnectorStatus::Running;
}

FStartRequest::FStartRequest()
{
}

void FStartRequest::FromProto(FStartRequest& EmptyResetRequest, const GymConnectorStartRequest& ProtoMsg)
{
	//Do nothing since the message currently contains no information
}

FStartRequest::FStartRequest(const GymConnectorStartRequest& ProtoMsg)
{
	FStartRequest::FromProto(*this, ProtoMsg);
}

FStartRequest* FStartRequest::FromProto(const GymConnectorStartRequest& ProtoMsg)
{
	return new FStartRequest(ProtoMsg);
}
