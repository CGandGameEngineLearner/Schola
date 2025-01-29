// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actuators/AbstractActuators.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "DebugActuators.generated.h"

/**
 * @brief A debug actuator that can be used for testing actuator discovery and spaces from python
 */
UCLASS(Blueprintable, BlueprintType)
class SCHOLA_API UDebugBoxActuator : public UBoxActuator
{
	GENERATED_BODY()
public:
	
	/** The Action Space to use in this Actuator */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBoxSpace ActionSpace = FBoxSpace();

	FBoxSpace GetActionSpace();

	/** 
	* @brief No-Op 
	* @param[in] Action A box point that is disgarded.
	*/
	void TakeAction(const FBoxPoint& Action) {};
};

/**
 * @brief A debug actuator that can be used for testing actuator discovery and spaces from python
 */
UCLASS( Blueprintable, BlueprintType)
class SCHOLA_API UDebugDiscreteActuator : public UDiscreteActuator
{
	GENERATED_BODY()
public:

	/** The Action Space to use in this Actuator */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDiscreteSpace ActionSpace;

	FDiscreteSpace GetActionSpace();

	void TakeAction(const FDiscretePoint& Action) {};
};

/**
 * @brief A debug actuator that can be used for testing actuator discovery and spaces from python
 */
UCLASS(Blueprintable, BlueprintType)
class SCHOLA_API UDebugBinaryActuator : public UBinaryActuator
{
	GENERATED_BODY()
public:

	/** The Action Space to use in this Actuator */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBinarySpace ActionSpace;

	FBinarySpace GetActionSpace();

	void TakeAction(const FBinaryPoint& Action){};
};