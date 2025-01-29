// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observers/AbstractObservers.h"
#include "RotationObserver.generated.h"

/**
 * @brief An observer that tracks the rotation of an actor
 */
UCLASS(Blueprintable)
class SCHOLA_API URotationObserver : public UBoxObserver
{
	GENERATED_BODY()

public:
	/** The Min/Max value for the Pitch of the tracked rotation */
	UPROPERTY(VisibleAnywhere, meta = (EditCondition = "bHasPitch"), Category = "Sensor Settings")
	FBoxSpaceDimension PitchBounds = FBoxSpaceDimension(-180, 180);

	/** The Min/Max value for the Roll of the tracked rotation */
	UPROPERTY(VisibleAnywhere, meta = (EditCondition = "bHasRoll"), Category = "Sensor Settings")
	FBoxSpaceDimension RollBounds = FBoxSpaceDimension(-180, 180);

	/** The Min/Max value for the Yaw of the tracked rotation */
	UPROPERTY(VisibleAnywhere, meta = (EditCondition = "bHasYaw"), Category = "Sensor Settings")
	FBoxSpaceDimension YawBounds = FBoxSpaceDimension(-180, 180);

	/** Should the observer track the Pitch of the rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasPitch = true;

	/** Should the observer track the Roll of the rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasRoll = true;

	/** Should the observer track the Yaw of the rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bHasYaw = true;

	/** The actor to track the rotation of, if None defaults to Owner of the Observer */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bTrackNonOwner", DisplayName = "Track Other Actor"), Category = "Sensor Settings")
	AActor* TrackedActor;

	/** Should the observer track the rotation of an actor other than the owner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (InlineEditConditionToggle), Category = "Sensor Settings")
	bool bTrackNonOwner = false;

	FBoxSpace GetObservationSpace() const;

	virtual void CollectObservations(FBoxPoint& OutObservations) override;
};
