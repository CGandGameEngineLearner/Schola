// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observers/AbstractObservers.h"
#include "Common/PositionalEnums.h"
#include "PositionObserver.generated.h"

/**
 * @brief An observer that tracks the position of an actor
 */
UCLASS(Blueprintable)
class SCHOLA_API UPositionObserver : public UBoxObserver
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

	/** Should the observer track the XDimension of the position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasXDimensions = true;

	/** Should the observer track the ZDimension of the position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasZDimensions = true;

	/** Should the observer track the YDimension of the position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasYDimensions = true;

	/** The actor to track the position of, if None defaults to Owner of the Observer */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bTrackNonOwner", DisplayName = "Track Other Actor"), Category = "Sensor Settings")
	AActor* TrackedActor;

	/** Should the observer track the position of an actor other than the owner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bTrackNonOwner = false;

	/** What frame of reference to use for this sensor, egocentric to the owning actor, relative to the owning actor or world coordinates*/
	UPROPERTY(EditAnywhere, Category = "Sensor Settings", meta = (EditCondition = "bTrackNonOwner", HideEditConditionToggle))
	EFrameOfReference PositionAdjustment = EFrameOfReference::World;

	FBoxSpace GetObservationSpace() const;

	virtual void CollectObservations(FBoxPoint& OutObservations) override;
};
