// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Communicator/CommunicationManager.h"

bool UCommunicationManager::RegisterService(std::shared_ptr<grpc::Service> Service)
{
	// Add this service if it doesn't already exist
	// We only track this for registration purposes so don't take ownership

	if (Service == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("Service is null Skipping"));
		return false;
	}
	else if (RegisteredServices.count(Service.get()) == 0)
	{
		UE_LOG(LogSchola, Log, TEXT("Service Registered"));
		Builder->RegisterService(Service.get());
		RegisteredServices.emplace(Service.get());
		return true;
	}
	else
	{
		UE_LOG(LogSchola, Warning, TEXT("Service Exists: Skipping Registration"));
		return false;
	}
}

std::unique_ptr<ServerCompletionQueue> UCommunicationManager::GetCompletionQueue()
{
	return Builder->AddCompletionQueue();
}

void UCommunicationManager::ShutdownServer()
{
	this->State = EComSystemState::NOTSTARTED;
	UE_LOG(LogSchola, Warning, TEXT("CLEANING UP BACKEND FACTORY??"));
	// Stop receiving RPC requests
	if (Server != nullptr)
	{
		// Shut the server down with a right now deadline
		Server->Shutdown(gpr_inf_past(GPR_CLOCK_MONOTONIC));
	}
	else
	{
		UE_LOG(LogSchola, Warning, TEXT("Server was Null"));
	}
	UE_LOG(LogSchola, Warning, TEXT("Server Shutdown: Closing CQueues"));
	// broadcast to all the backends to clean up their resources
	OnServerShutdownDelegate.Broadcast();
	UE_LOG(LogSchola, Warning, TEXT("All CQueues Closed"));
}

UCommunicationManager::~UCommunicationManager()
{
	if (this->State == EComSystemState::STARTED)
	{
		// Keep these seperate incase we want to make this restartable
		this->ShutdownServer();
	}
	delete this->Builder;
	this->Builder = nullptr;
}

bool UCommunicationManager::StartBackends()
{

	Server = Builder->BuildAndStart();
	if (Server == nullptr)
	{
		UE_LOG(LogSchola, Warning, TEXT("Server not started. Address %s Unavailable or not all services were starter."), *ServerURL);
		this->State = EComSystemState::FAILURE;
		return false;
	}
	else
	{
		UE_LOG(LogSchola, Log, TEXT("Running Server on: %s"), *ServerURL);

		// Perform initialization of the server
		this->OnServerStartDelegate.Broadcast();
		this->State = EComSystemState::STARTED;

		// Call Each Communication Interfaces Establish methods
		this->OnServerReadyDelegate.Broadcast();

		// Send any initial messages (e.g. Space Definitions)
		this->OnConnectionEstablishedDelegate.Broadcast();

		return true;
	}
}

void UCommunicationManager::Initialize()
{
	// Load settings from our corresponding developer settings object
	const UScholaManagerSubsystemSettings* Settings = GetDefault<UScholaManagerSubsystemSettings>();
	int							  Port;
	if (!FParse::Value(FCommandLine::Get(), TEXT("ScholaPort"), Port))
	{
		// Parse failed so we fall back to the default
		Port = Settings->CommunicatorSettings.Port;
	}
	this->ServerURL = Settings->CommunicatorSettings.Address + FString(":") + FString::FromInt(Port);
	Builder = new grpc::ServerBuilder();
	Builder->AddListeningPort(TCHAR_TO_UTF8(*ServerURL), grpc::InsecureServerCredentials());
}

