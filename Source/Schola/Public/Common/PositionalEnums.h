// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * @brief Enum class of frame of reference types.
 */
UENUM()
enum class EFrameOfReference
{
	/** Rotation and Position are both relative to a target Actor. Useful for agents that primarily navigate forward */
	Egocentric,
	/** Position is relative to a target actor. */
	Relative,
	/** Position is based on world position. */
	World
};