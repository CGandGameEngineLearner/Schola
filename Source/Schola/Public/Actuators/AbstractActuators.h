// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Common/Spaces.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/AbstractInteractor.h"
#include "AbstractActuators.generated.h"

/**
 * @brief An actuator is a component that can be attached to an agent to make actions. It is the interface between the agent and the environment.
 * @details An actuator can be of different types, such as movement, shooting, etc. It can be continuous, discrete, or binary.
 */
UCLASS(Abstract, EditInlineNew)
class SCHOLA_API UActuator : public UAbstractInteractor
{
	GENERATED_BODY()
public:

	/**
	* @brief Get the Space bounding the inputs to this actuator
	* @param[out] OutSpaceGroup An empty SpaceVariant that will be filled with the ActionSpace of this Actuator
	*/
	virtual void FillActionSpace(TSpace& OutSpaceGroup) PURE_VIRTUAL(UActuator::FillActionSpace, return; );

	/**
	 * @brief Use this actuator to take an action impacting the world
	 * @param[in] Action PointGroup containing the inputs to this actuator from the brain
	 */
	virtual void TakeAction(const TPoint& Action) PURE_VIRTUAL(UActuator::TakeAction, return; );

	/**
	* @brief Helper function to spawn a child actor, since the builtin method is not available in UObjects.
	* @note Will cause an error if called from a UObserver that isn't part of the world.
	*/
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf, HideSelfPin, DeterminesOutputType = "Class", AdvancedDisplay = "TransformScaleMethod,Owner"), Category = "Actuator Utilities")
	AActor* SpawnActor(TSubclassOf<AActor> Class, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator);

	/**
	 * @brief Do any subclass specific setup.
	 * @note This function should be implemented by any derived classes
	 */
	virtual void InitializeActuator(){};


#if WITH_EDITOR

	/**
	 * @brief Set the debug actions for this observer.
	 * @param[in] Temp The temporary point to copy to the debug actions.
	 * @note This function should be implemented by any derived classes, to use the correct type of point.
	 */
	virtual void SetDebugActions(const TPoint& Temp) PURE_VIRTUAL(UAbstractActuator::SetDebugActions, return;);
#endif

};

/**
 * @brief An actuator that does continuous actions, and takes inputs bounded by a box space.
 */
UCLASS(Abstract, BlueprintType)
class SCHOLA_API UBoxActuator : public UActuator
{
	GENERATED_BODY()
public:
	/**
	* @brief Get the Space bounding the inputs to this actuator
	* @return BoxSpace containing the bounds of the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual FBoxSpace GetActionSpace() PURE_VIRTUAL(UBoxActuator::GetActionSpace, return FBoxSpace(););

	/**
	* @brief Use this actuator to take an action impacting the world.
	* @param[in] Action BoxPoint containing the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual void TakeAction(const FBoxPoint& Action) PURE_VIRTUAL(UBoxActuator::TakeAction, return; );

	void TakeAction(const TPoint& Action) override
	{
	#if WITH_EDITOR
		this->SetDebugActions(Action);
	#endif
		this->TakeAction(Action.Get<FBoxPoint>());
	}

	void FillActionSpace(TSpace& OutSpace) override
	{
		OutSpace.Set<FBoxSpace>(this->GetActionSpace());
	}

#if WITH_EDITOR
	void SetDebugActions(const TPoint& Temp) override;
#endif

#if WITH_EDITORONLY_DATA
	/** The debug actions for this actuator. Shows the last taken action  */
	UPROPERTY(VisibleInstanceOnly, Category = "Actuator Utilities")
	TArray<float> DebugBoxPoint;
#endif
};

/**
 * @brief An actuator that does discrete actions
 */
UCLASS(Abstract, BlueprintType)
class SCHOLA_API UDiscreteActuator : public UActuator
{
	GENERATED_BODY()

public:
	/**
	* @brief Get the Space bounding the inputs to this actuator
	* @return DiscreteSpace containing the bounds of the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual FDiscreteSpace GetActionSpace() PURE_VIRTUAL(UDiscreteActuator::GetActionSpace, return FDiscreteSpace(););

	/**
	* @brief Use this actuator to take an action impacting the world
	* @param[in] Action DiscretePoint containing the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual void TakeAction(const FDiscretePoint& Action) PURE_VIRTUAL(UDiscreteActuator::TakeAction, return; );

	void TakeAction(const TPoint& Action) override
	{	
	#if WITH_EDITOR
		this->SetDebugActions(Action);
	#endif
		this->TakeAction(Action.Get<FDiscretePoint>());
	}

	void FillActionSpace(TSpace& OutSpace) override
	{
		OutSpace.Set<FDiscreteSpace>(this->GetActionSpace());
	}


#if WITH_EDITOR
	void SetDebugActions(const TPoint& Temp) override;
#endif

#if WITH_EDITORONLY_DATA
	/** The debug actions for this actuator. Shows the last taken action  */
	UPROPERTY(VisibleInstanceOnly, Category = "Actuator Utilities")
	TArray<int> DebugDiscretePoint;
#endif

};

/**
 * @brief An actuator that does binary actions
 */
UCLASS(Abstract, BlueprintType)
class SCHOLA_API UBinaryActuator : public UActuator
{
	GENERATED_BODY()

public:
	/**
	* @brief Get the Space bounding the inputs to this actuator
	* @return BinarySpace containing the bounds of the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual FBinarySpace GetActionSpace() PURE_VIRTUAL(UBinaryActuator::GetActionSpace, return FBinarySpace(););
	
	/**
	* @brief Use this actuator to take an action impacting the world
	* @param[in] Action BinaryPoint containing the inputs to this actuator
	* @note This function must be implemented in the derived class.
	*/
	virtual void TakeAction(const FBinaryPoint& Action) PURE_VIRTUAL(UBinaryActuator::TakeAction, return; );

	void TakeAction(const TPoint& Action) override
	{
		#if WITH_EDITOR
			this->SetDebugActions(Action);
		#endif
		this->TakeAction(Action.Get<FBinaryPoint>());
	}

	void FillActionSpace(TSpace& OutSpace) override
	{
		OutSpace.Set<FBinarySpace>(this->GetActionSpace());
	}

#if WITH_EDITOR
	void SetDebugActions(const TPoint& Temp) override;
#endif

#if WITH_EDITORONLY_DATA
	/** The debug actions for this observer. Shows the last taken action  */
	UPROPERTY(VisibleInstanceOnly, Category = "Actuator Utilities")
	TArray<bool> DebugBinaryPoint;
#endif

};

/**
 * @brief Blueprint version of box actuator
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SCHOLA_API UBlueprintBoxActuator : public UBoxActuator
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	FBoxSpace GetActionSpace() override;

	UFUNCTION(BlueprintImplementableEvent)
	void TakeAction(const FBoxPoint& Action) override;

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeActuator() override;

};

/**
 * @brief Blueprint version of discrete actuator
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SCHOLA_API UBlueprintDiscreteActuator : public UDiscreteActuator
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	FDiscreteSpace GetActionSpace() override;

	UFUNCTION(BlueprintImplementableEvent)
	void TakeAction(const FDiscretePoint& Action) override;

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeActuator() override;
};

/**
 * @brief Blueprint version of binary actuator
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SCHOLA_API UBlueprintBinaryActuator : public UBinaryActuator
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	FBinarySpace GetActionSpace() override;

	UFUNCTION(BlueprintImplementableEvent)
	void TakeAction(const FBinaryPoint& Action) override;

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeActuator() override;
};