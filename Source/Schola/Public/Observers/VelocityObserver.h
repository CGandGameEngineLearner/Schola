// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observers/AbstractObservers.h"
#include "VelocityObserver.generated.h"

/**
 * @brief An observer that tracks the velocity of an actor
 */
UCLASS(Blueprintable)
class SCHOLA_API UVelocityObserver : public UBoxObserver
{
	GENERATED_BODY()

public:
	/** The Min/Max value for the XDimension of the tracked position */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasXDimensions"), Category = "Sensor Settings")
	FBoxSpaceDimension XDimensionBounds;

	/** The Min/Max value for the YDimension of the tracked position */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasYDimensions"), Category = "Sensor Settings")
	FBoxSpaceDimension YDimensionBounds;

	/** The Min/Max value for the ZDimension of the tracked position */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasZDimensions"), Category = "Sensor Settings")
	FBoxSpaceDimension ZDimensionBounds;

	/** Should the observer track the XDimension of the velocity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasXDimensions = true;

	/** Should the observer track the ZDimension of the velocity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasZDimensions = true;

	/** Should the observer track the YDimension of the velocity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasYDimensions = true;

	/** The actor to track the velocity of, if None defaults to Owner of the Observer */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bTrackNonOwner", DisplayName = "Track Other Actor"), Category = "Sensor Settings")
	AActor* TrackedActor;

	/** Should the observer track the velocity of an actor other than the owner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bTrackNonOwner = false;

	FBoxSpace GetObservationSpace() const;

	virtual void CollectObservations(FBoxPoint& OutObservations) override;
};
