// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "InteractionDefinition.generated.h"

/**
 * @brief Struct containing a definition of the inputs and outputs of a policy
 */
USTRUCT()
struct SCHOLA_API FInteractionDefinition
{

	GENERATED_BODY()

	/** Defines the range of values that the corresponding agent accepts as observations */
	UPROPERTY()
	FDictSpace ObsSpaceDefn;

	/* Defines the range of values that are output by this agent's policy as actions. */
	UPROPERTY()
	FDictSpace ActionSpaceDefn;

	/** Should observations be normalized. Only affects continuous observations. */
	UPROPERTY(EditAnywhere, Category = "Reinforcement Learning")
	bool bNormalizeObservations = false;

	FInteractionDefinition(const FInteractionDefinition& Other)
	{
		ObsSpaceDefn = Other.ObsSpaceDefn;
		ActionSpaceDefn = Other.ActionSpaceDefn;
		bNormalizeObservations = Other.bNormalizeObservations;
	}

	FInteractionDefinition()
	{

	}

};





