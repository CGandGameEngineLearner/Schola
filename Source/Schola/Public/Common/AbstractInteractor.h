// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbstractInteractor.generated.h"

/**
 * @brief An abstract class objects that can interact with the environment, either by observing or acting
 */
UCLASS(Abstract, EditInlineNew)
class UAbstractInteractor : public UObject
{
	GENERATED_BODY()
public:

	/** 
	 * @brief Try and Get the Actor this Observer is attached to, may return nullptr
	 * @return the Agent this Observer is attached to in, may return nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactor Utilities")
	AActor* TryGetOwner() const;
	
	/**
	 * @brief Get an non-unique label of this Interactor, based on the class of the object containing the interactor
	 * @return the collected ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactor Utilities")
	FString GetLabel() const;
	
	/**
	 * @brief Get the label of this Interactor, with a prepended numberical ID to ensure uniqueness and alphanumeric ordering.
	 * @return the collected ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactor Utilities")
	FString GetId(int IntId) const;

	/**
	* @brief Get an Outer for this object that is unique w.r.t the agent for getting the ID.
	* @return a UObject ptr to an owning object in the heirarchy
	* @note If the UObject is inside a Controller returns the controller. If the UObject is inside a Component return the owner of the component.
	*/
	UFUNCTION(BlueprintCallable, Category = "Interactor Utilities")
	UObject* GetLocation() const;

};