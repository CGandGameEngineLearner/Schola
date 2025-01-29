// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/LogSchola.h"
#include "Communicator/ComBackendInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/SubsystemSettings.h"
#include "../Communicator/ComBackendInterface.h"
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_context.h>
#include "grpcpp/server_builder.h"
#include <google/protobuf/map.h>
#include <google/protobuf/message.h>
#include "Communicator/ExchangeRPCBackend.h"
#include "Communicator/PollingRPCBackend.h"
#include "Communicator/ProducerRPCBackend.h"
#include "CommunicationManager.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnServerStartSignature);
DECLARE_MULTICAST_DELEGATE(FOnServerReadySignature);
DECLARE_MULTICAST_DELEGATE(FOnServerShutdownSignature);

/**
 * @brief An enumeration representing the state of the communication system
 */
UENUM()
enum class EComSystemState
{
	NOTSTARTED = 0,
	STARTED = 1,
	FAILURE = 3 // Not Used currently
};

/**
 * @brief A class that manages a gRPC server running on a specified URL
 */
UCLASS()
class SCHOLA_API UCommunicationManager : public UObject
{
	GENERATED_BODY()

private:
	/** The URL of the server */
	UPROPERTY()
	FString ServerURL;

	/** The server builder object */
	grpc::ServerBuilder*	Builder = nullptr;

	/** The server object */
	std::unique_ptr<Server> Server;

	/** list of existing services to prevent readding services */
	std::unordered_set<grpc::Service*> RegisteredServices;

	/** The state of the communication system */
	UPROPERTY()
	EComSystemState State = EComSystemState::NOTSTARTED;

	/**
	 * @brief  A type representing an Async RPC Handle 
	 * @tparam ServiceType The type of the service
	 * @tparam In The type of the input message
	 * @tparam Out The type of the output message
	*/
	template <typename ServiceType, typename In, typename Out>
	using AsyncRPCHandle = void (ServiceType::*)(grpc::ServerContext* context,
		In*															  request,
		ServerAsyncResponseWriter<Out>*								  response,
		grpc::CompletionQueue*										  new_call_cq,
		ServerCompletionQueue*										  notification_cq,
		void*														  tag);
	

public:
	/** A delegate that is called when the server starts */
	FOnServerStartSignature	   OnServerStartDelegate;
	/** A delegate that is called when the server is ready */
	FOnServerReadySignature	   OnServerReadyDelegate;
	/** A delegate that is called when the server establishes a connection */
	FOnServerReadySignature	   OnConnectionEstablishedDelegate;
	/** A delegate that is called when the server shuts down */
	FOnServerShutdownSignature OnServerShutdownDelegate;

	

	/**
	 * @brief Register a service with the server
	 * @param[in] Service The service to register
	 * @return True if the service was registered successfully, false otherwise
	 */
	bool RegisterService(std::shared_ptr<grpc::Service> Service);

	
	/**
	 * @brief Get the completion queue for the server
	 * @return The completion queue
	 */
	std::unique_ptr<ServerCompletionQueue> GetCompletionQueue();

	/**
	 * @brief Create a new Polling Backend, where Unreal Polls for messages of type In
	 * @tparam ServiceType The type of the service
	 * @tparam In The type of the input message
	 * @tparam Out The type of the output message
	 * @param TargetRPC The RPC method to wrap
	 * @param Service The service to attach the backend to
	 */
	template <typename ServiceType, typename In, typename Out>
	IPollingBackendInterface<In>* CreatePollingBackend(AsyncRPCHandle<ServiceType, In, Out> TargetRPC, std::shared_ptr<ServiceType> Service)
	{
		this->RegisterService(Service);
		std::unique_ptr<ServerCompletionQueue> CompQueue = this->GetCompletionQueue();
		IPollingBackendInterface<In>*		   Backend = new PollingRPCBackend<ServiceType, In, Out>(TargetRPC, Service, std::move(CompQueue));
		this->OnServerStartDelegate.AddRaw(Backend, &IComBackendInterface::Start);
		this->OnServerShutdownDelegate.AddRaw(Backend, &IComBackendInterface::Shutdown);
		return Backend;
	}

	/**
	 * @brief Create a new Producer Backend, where Unreal Sends messages of type Out
	 * @tparam ServiceType The type of the service
	 * @tparam In The type of the input message
	 * @tparam Out The type of the output message
	 * @param TargetRPC The RPC method to wrap
	 * @param Service The service to attach the backend to
	 */
	template <typename ServiceType, typename In, typename Out>
	IProducerBackendInterface<Out>* CreateProducerBackend(AsyncRPCHandle<ServiceType, In, Out> TargetRPC, std::shared_ptr<ServiceType> Service)
	{
		this->RegisterService(Service);
		std::unique_ptr<ServerCompletionQueue> CompQueue = this->GetCompletionQueue();
		IProducerBackendInterface<Out>*		   Backend = new ProducerRPCBackend<ServiceType, In, Out>(TargetRPC, Service, std::move(CompQueue));
		this->OnServerStartDelegate.AddRaw(Backend, &IComBackendInterface::Start);
		this->OnServerReadyDelegate.AddRaw(Backend, &IProducerBackendInterface<Out>::Establish);
		this->OnServerShutdownDelegate.AddRaw(Backend, &IComBackendInterface::Shutdown);
		return Backend;
	}
	
	/**
	 * @brief Create a new Exchange Backend, where Unreal Sends messages of type Out and receives back messages of type In
	 * @tparam ServiceType The type of the service
	 * @tparam In The type of the input message
	 * @tparam Out The type of the output message
	 * @param TargetRPC The RPC method to wrap
	 * @param Service The service to attach the backend to
	 */
	template <typename ServiceType, typename In, typename Out>
	IExchangeBackendInterface<In, Out>* CreateExchangeBackend(AsyncRPCHandle<ServiceType, In, Out> TargetRPC, std::shared_ptr<ServiceType> Service)
	{
		this->RegisterService(Service);
		std::unique_ptr<ServerCompletionQueue> CompQueue = this->GetCompletionQueue();
		IExchangeBackendInterface<In, Out>*	   Backend = new ExchangeRPCBackend<ServiceType, In, Out>(TargetRPC, Service, std::move(CompQueue));
		this->OnServerStartDelegate.AddRaw(Backend, &IComBackendInterface::Start);
		this->OnServerShutdownDelegate.AddRaw(Backend, &IComBackendInterface::Shutdown);
		return Backend;
	}

	/**
	 * @brief Shutdown the Communication manager. Triggers OnServerShutdownDelegate
	 */
	void ShutdownServer();

	/**
	 * @brief Starts all backends created by the communication manager. Triggers OnServerStartDelegate and OnServerReadyDelegate
	 */
	bool StartBackends();

	~UCommunicationManager();

	/**
	 * @brief Initialize the Communication Manager. Sets the Port and URL for the server from the Settings Panel
	 */
	void Initialize();
};
