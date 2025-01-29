// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Actuators/AbstractActuators.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Agent/AgentComponents/InteractionComponent.h"
#include "ActuatorComponent.generated.h"

/**
* @brief An ActorComponent for holding onto an Actuator and providing some related utility functions
*/
UCLASS(ClassGroup = Schola, meta = (BlueprintSpawnableComponent))
class SCHOLA_API UActuatorComponent : public UInteractionComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UActuatorComponent();

	/** The Actuator Object inside this Actuator Component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Instanced, meta = (ShowInnerProperties))
	UActuator* Actuator;

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

public:
	/** Called every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
