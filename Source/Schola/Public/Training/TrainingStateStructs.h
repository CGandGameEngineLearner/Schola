// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "Common/LogSchola.h"
#include "Containers/SortedMap.h"
THIRD_PARTY_INCLUDES_START
#include "../Generated/GymConnector.pb.h"
THIRD_PARTY_INCLUDES_END
#include "Communicator/ProtobufSerializer.h"
#include "TrainingStateStructs.generated.h"

/**
 * @brief An enumeration representing the various states common across gym and gym-like environments. Typically use Running, Truncated and Completed
 */
UENUM(BlueprintType)
enum class EAgentTrainingStatus : uint8
{
	Running	  UMETA(DisplayName = "Running"),
	Truncated UMETA(DisplayName = "Truncated"),
	Completed UMETA(DisplayName = "Completed"),
};
/**
 * @brief An enumeration representing the status of the training message
 */
UENUM(BlueprintType)
enum class ETrainingMsgStatus : uint8
{
	NoStatus	   UMETA(DisplayName = "None"),
	LastMsgPending UMETA(DisplayName = "Pending"),
	LastMsgSent	   UMETA(DisplayName = "Sent"),
};

/**
 * @brief A Struct representing the state of the agent given by a Reward, a vector observation and a status
 */
USTRUCT(BlueprintType)
struct FTrainerState
{
	GENERATED_BODY()

	/** The reward received by the agent in the last step */
	UPROPERTY(BlueprintReadOnly)
	float Reward = 0.0;

	/** Whether we have sent out our last message after completing an episode */
	UPROPERTY(BlueprintReadOnly)
	ETrainingMsgStatus TrainingMsgStatus = ETrainingMsgStatus::NoStatus;

	/** The vector observation of the agent in the last step. Not a UProperty because FDictPoint is not blueprintable */
	FDictPoint* Observations;

	/** A map of key-value pairs containing additional information about the agent from the last step*/
	UPROPERTY()
	TMap<FString,FString> Info;

	/** The status of the agent in the last step */
	UPROPERTY(BlueprintReadOnly)
	EAgentTrainingStatus TrainingStatus = EAgentTrainingStatus::Running;

	/**
	 * @brief Fill a protobuf message (Schola::AgentState) with the agent's state
	 * @param[out] OutState The protobuf message reference to fill
	 */
	void ToProto(Schola::AgentState& OutState) const
	{
		ProtobufSerializer Serializer = ProtobufSerializer(OutState.mutable_observations());

		Observations->Accept(Serializer);
		
		for (auto& InfoEntry : this->Info)
		{
			(*OutState.mutable_info())[TCHAR_TO_UTF8(*InfoEntry.Key)] = TCHAR_TO_UTF8(*InfoEntry.Value);
		}

		OutState.set_reward(this->Reward);

		// convert from Unreal visible enum to gRPC enum
		switch (this->TrainingStatus)
		{
			case EAgentTrainingStatus::Running:
				OutState.set_status(Schola::RUNNING);
				break;
			case EAgentTrainingStatus::Completed:
				OutState.set_status(Schola::COMPLETED);
				break;
			case EAgentTrainingStatus::Truncated:
				OutState.set_status(Schola::TRUNCATED);
				break;
		}
	}

	/**
	 * @brief Fill a protobuf message (Schola::AgentState) with the agent's state
	 * @param[out] OutState The protobuf message ptr to fill
	 */
	void ToProto(Schola::AgentState* OutState) const
	{
		return this->ToProto(*OutState);
	}

	/**
	 * @brief Convert this object to a protobuf message (Schola::AgentState)
	 * @return A new protobuf message containing the contents of this object
	 */
	Schola::AgentState* ToProto() const
	{
		Schola::AgentState* AgentStateMsg = new Schola::AgentState();
		this->ToProto(AgentStateMsg);
		return AgentStateMsg;
	}

	/**
	 * @brief Is this agent done the current episode of training.
	 * @return true iff the agent is done it's current episode.
	 */
	bool IsDone() const
	{
		return TrainingStatus == EAgentTrainingStatus::Completed || TrainingStatus == EAgentTrainingStatus::Truncated;
	}

	/**
	 * @brief Create a protobuf message (Schola::InitialAgentState) corresponding to the initial state of the agent after a reset.
	 * @param[out] OutState The protobuf message reference to fill
	 */
	void ToResetProto(Schola::InitialAgentState& OutState) const
	{
		ProtobufSerializer Visitor = ProtobufSerializer(OutState.mutable_observations());
		Observations->Accept(Visitor);
		
		for (auto& InfoEntry : this->Info)
		{
			(*OutState.mutable_info())[TCHAR_TO_UTF8(*InfoEntry.Key)] = TCHAR_TO_UTF8(*InfoEntry.Value);
		}
		
	}

};

/**
 * @brief A Struct representing the state of an environment given by a set of AgentStates
 */
USTRUCT(BlueprintType)
struct FSharedEnvironmentState
{

	GENERATED_BODY()

	/** Map from AgentId to AgentState */
	TSortedMap<int, FTrainerState*> AgentStates;

	/** 
	 * @brief Default constructor for FSharedEnvironmentState
	*/
	FSharedEnvironmentState(){};

	/**
	 * @brief Add a shared agent state to the shared environment state
	 * @param[in] AgentId The key to add the agent state under
	 * @param[in,out] SharedStatePointer The shared agent state to add
	 */
	void AddSharedAgentState(int AgentId, FTrainerState* SharedStatePointer)
	{
		this->AgentStates.Add(AgentId, SharedStatePointer);
	}

	/**
	 * @brief Fill a protobuf message (Schola::EnvironmentState) with the contents of this object
	 * @param[out] OutMsg A reference to the protobuf message to fill
	 */
	void ToProto(Schola::EnvironmentState& OutMsg) const
	{

		for (const TPair<int, FTrainerState*>& IdToSharedState : AgentStates)
		{

			if (IdToSharedState.Value->TrainingMsgStatus == ETrainingMsgStatus::LastMsgSent)
			{
				continue;
			}

			Schola::AgentState& AgentStateMsg = (*OutMsg.mutable_agent_states())[IdToSharedState.Key];
			IdToSharedState.Value->ToProto(AgentStateMsg);
		}
	}

	/**
	 * @brief Fill a protobuf message (Schola::EnvironmentState) with the contents of this object
	 * @param[out] OutMsg A pointer to the protobuf message to fill
	 */
	void ToProto(Schola::EnvironmentState* OutMsg) const
	{
		return this->ToProto(*OutMsg);
	}

	/**
	 * @brief Convert this object to a protobuf message (Schola::EnvironmentState)
	 * @return A new protobuf message containing the contents of this object
	 */
	Schola::EnvironmentState* ToProto() const
	{
		Schola::EnvironmentState* EnvironmentStateMessage = new Schola::EnvironmentState();
		this->ToProto(EnvironmentStateMessage);
		return EnvironmentStateMessage;
	}

	/**
	 * @brief Fill a protobuf message (Schola::InitialEnvironmentState) with the initial state of the environment after a reset.
	 * @param[out] OutTrainingStateMessage The protobuf message reference to fill
	 */
	void ToResetProto(Schola::InitialEnvironmentState& OutTrainingStateMessage) const
	{
		for (const TPair<int, FTrainerState*>& AgentIdToState : AgentStates)
		{
			AgentIdToState.Value->ToResetProto((*OutTrainingStateMessage.mutable_agent_states())[AgentIdToState.Key]);
		}
	}
};

/**
 * @brief A Struct representing the state of the training session given by a set of EnvironmentStates
 */
USTRUCT(BlueprintType)
struct FTrainingState
{
	GENERATED_BODY()

	/** Map from EnvironmentId to EnvironmentState */
	TArray<FSharedEnvironmentState> EnvironmentStates;

	FTrainingState(){};

	/**
	 * @brief Convert this object to a protobuf message (Schola::TrainingState)
	 * @return A new protobuf message containing the contents of this object
	 */
	Schola::TrainingState* ToProto() const
	{
		Schola::TrainingState* TrainingStateMessage = new Schola::TrainingState();
		for (const FSharedEnvironmentState& EnvState : EnvironmentStates)
		{
			// Fill the mappings
			EnvState.ToProto((*TrainingStateMessage->add_environment_states()));
		}

		return TrainingStateMessage;
	}

	/**
	 * @brief Convert this object to a protobuf message (Schola::InitialTrainingState) representing the initial state of a subset of environments after a reset.
	 * @param[in] TargetEnvironments The list of environment ids to include in the message
	 * @return A new protobuf message containing the initial state of the specified environments
	 */
	Schola::InitialTrainingState* ToResetProto(const TArray<int>& TargetEnvironments) const
	{
		Schola::InitialTrainingState* TrainingStateMessage = new Schola::InitialTrainingState();
		for (int EnvId : TargetEnvironments)
		{
			const FSharedEnvironmentState& EnvState = EnvironmentStates[EnvId];
			// Fill the mappings
			EnvState.ToResetProto((*TrainingStateMessage->mutable_environment_states())[EnvId]);
		}

		return TrainingStateMessage;
	}
};
