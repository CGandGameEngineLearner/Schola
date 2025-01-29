// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "Common/InteractionDefinition.h"
THIRD_PARTY_INCLUDES_START
#include "../Generated/Definitions.pb.h"
THIRD_PARTY_INCLUDES_END
#include "TrainingDefinitionStructs.generated.h"

/**
 * @brief Struct containing the Id of an agent.
 */
USTRUCT()
struct SCHOLA_API FAgentId
{
	GENERATED_BODY()
	
	/** The unique identifier of the agent with respect to it's environment */
	UPROPERTY()
	int AgentId;

	/** The unique identifier of the environment the agent is in. */
	UPROPERTY()
	int EnvId;

	/**
	 * @brief Construct a new default FAgentId object
	 */
	FAgentId()
	{
		AgentId = 0;
		EnvId = 0;
	}

	/**
	 * @brief Construct a new FAgentId object
	 * @param AgentId The unique identifier of the agent
	 * @param EnvId The unique identifier of the environment the agent is in
	 */
	FAgentId(int AgentId, int EnvId)
	{
		this->AgentId = AgentId;
		this->EnvId = EnvId;
	}

	/**
	 * @brief Copy construct a new FAgentId object
	 * @param Other An existing FAgentId object to copy
	 */
	FAgentId(const FAgentId& Other)
	{
		this->AgentId = Other.AgentId;
		this->EnvId = Other.EnvId;
	}
};


/**
 * @brief Struct containing the properties that define an agent.
*/
USTRUCT()
struct SCHOLA_API FTrainerDefinition
{

	GENERATED_BODY()

	/* The unique identifier of this agent. */
	UPROPERTY()
	int UniqueId = -1;

	/* The two part Id of the agent. */
	UPROPERTY()
	FAgentId Id;

	/* The name of this agent. Used for logging etc. Is not a unique identifier. */
	UPROPERTY(EditAnywhere, Category = "Reinforcement Learning")
	FString Name;

	/* The policy definition, stored and linked in the interaction manager */
	FInteractionDefinition* PolicyDefinition;

	/**
	 * @brief Construct a new FTrainerDefinition object
	 * @note This is required for the Unreal Engine reflection system to avoid C4239 and C2512 errors
	 */
	FTrainerDefinition()
	{
		// Just make a definition with default values
	}
	/**
	 * @brief Copy construct a new FTrainerDefinition object
	 * @param[in] Other An existing FTrainerDefinition object to copy
	 */
	FTrainerDefinition(const FTrainerDefinition& Other)
	{
		UniqueId = Other.UniqueId;
		Name = Other.Name;
		Id = Other.Id;
		PolicyDefinition = Other.PolicyDefinition;
	}

	/**
	 * @brief Fill a protobuf message (Schola::AgentDefinition) with the contents of this object
	 * @param[out] Msg The protobuf message to fill
	 */
	void ToProtobuf(Schola::AgentDefinition* Msg) const
	{

		this->PolicyDefinition->ActionSpaceDefn.FillProtobuf(Msg->mutable_action_space());
		this->PolicyDefinition->ObsSpaceDefn.FillProtobuf(Msg->mutable_obs_space());

		Msg->set_normalize_obs(this->PolicyDefinition->bNormalizeObservations);

		// Placeholder for potential action normalization later.
		Msg->set_normalize_actions(false);

		Msg->set_name(std::string(TCHAR_TO_UTF8(*Name)));
	}
	/**
	 * @brief Convert this object to a protobuf message (Schola::AgentDefinition)
	 * @return A new protobuf message containing the contents of this object
	 */
	Schola::AgentDefinition* ToProtobuf() const
	{
		Schola::AgentDefinition* Msg = new Schola::AgentDefinition();
		this->ToProtobuf(Msg);
		return Msg;
	}
};

/**
 * @brief Struct containing the properties that define an environment. Shared between the GymConnector and the Environment Objects.
 */
USTRUCT()
struct SCHOLA_API FSharedEnvironmentDefinition
{
	GENERATED_BODY()

	/** Map from Environment Name,Agent Name to Agent Definitions */
	TSortedMap<int, FTrainerDefinition*> AgentDefinitions;

	/**
	 * @brief Fill a protobuf message (Schola::EnvironmentDefinition) with the contents of this object
	 * @param[out] Msg The protobuf message to fill
	 */
	void ToProtobuf(Schola::EnvironmentDefinition* Msg) const
	{
		for (const TTuple<int, FTrainerDefinition*>& IdToAgentDefn : AgentDefinitions)
		{
			Schola::AgentDefinition AgentDefnMessage;
			IdToAgentDefn.Value->ToProtobuf(&AgentDefnMessage);
			(*Msg->mutable_agent_definitions())[IdToAgentDefn.Key] = AgentDefnMessage;
		}
	}

	/**
	 * @brief Add a shared agent definition to the shared environment definition
	 * @param[in] Key The key to add the agent definition under
	 * @param[in] SharedDefnPointer The shared agent definition to add
	 */
	void AddSharedAgentDefn(int Key, FTrainerDefinition* SharedDefnPointer)
	{
		this->AgentDefinitions.Add(Key, SharedDefnPointer);
	}

};

/**
 * @brief Struct containing the properties that define a training session.
 */
USTRUCT()
struct SCHOLA_API FSharedTrainingDefinition
{
	GENERATED_BODY()
	/** Map from EnvironmentID to Environment Definition, which is itself a Map from AgentId to AgentDefinition */
	UPROPERTY()
	TArray<FSharedEnvironmentDefinition> EnvironmentDefinitions;

	/**
	 * @brief Fill a protobuf message (Schola::TrainingDefinition) with the contents of this object
	 * @return A new protobuf message containing the contents of this object
	 */
	Schola::TrainingDefinition* ToProtobuf() const
	{
		Schola::TrainingDefinition* Msg = new Schola::TrainingDefinition();
		for (const FSharedEnvironmentDefinition& EnvDefn : EnvironmentDefinitions)
		{
			Schola::EnvironmentDefinition* EnvDefnMessage = Msg->add_environment_definitions();
			EnvDefn.ToProtobuf(EnvDefnMessage);
		}

		return Msg;
	}
};