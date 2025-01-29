// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <Kismet/GameplayStatics.h>
#include "Common/LogSchola.h"
#include "./IInferenceAgent.h"
#include "InferencePawn.generated.h"

/**
 * @brief A pawn that implements the IInferenceAgent interface.
 */
UCLASS()
class SCHOLA_API AInferencePawn : public APawn, public IInferenceAgent
{
	GENERATED_BODY()

public:
	/** Object defining how the agent interacts with the environment. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	UInteractionManager* InteractionManager = CreateDefaultSubobject<UInteractionManager>(TEXT("InteractionManager"));

	/** Object defining an asynchronous function f:Observations->Actions used to make decisions for the agent. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	UAbstractPolicy* Policy;

	/** Object defining how decisions requests are synchronized. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	UAbstractBrain* Brain;

	/** List of observers that collect observations for the agent. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	TArray<UAbstractObserver*> Observers;

	/** List of actuators that execute actions for the agent. */
	UPROPERTY(EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties), Category = "Reinforcement Learning")
	TArray<UActuator*> Actuators;

	/** The status of the agent. */
	UPROPERTY(BlueprintReadOnly)
	EAgentStatus Status = EAgentStatus::Running;

	virtual APawn* GetControlledPawn() override
	{
		return this;
	}
	virtual UInteractionManager* GetInteractionManager() override
	{
		return InteractionManager;
	}
	virtual UAbstractBrain* GetBrain() override
	{
		return Brain;
	}
	virtual UAbstractPolicy* GetPolicy() override
	{
		return Policy;
	}
	virtual TArray<UAbstractObserver*> GetAllObservers() override
	{
		TArray<UAbstractObserver*> AllObservers;
		AllObservers.Append(GetObserversFromPawn());
		AllObservers.Append(Observers);
		return AllObservers;
	}
	virtual TArray<UActuator*> GetAllActuators() override
	{
		TArray<UActuator*> AllActuators;
		AllActuators.Append(GetActuatorsFromPawn());
		AllActuators.Append(Actuators);
		return AllActuators;
	}

	virtual EAgentStatus GetStatus() override
	{
		return Status;
	}

	virtual void SetStatus(EAgentStatus NewStatus) override
	{
		Status = NewStatus;
	}
};