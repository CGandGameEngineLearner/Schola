// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/LogSchola.h"
#include "Observers/AbstractObservers.h"
#include "RayCastObserver.generated.h"

/**
 * @brief An observer that casts rays and collects observations about the first object hit.
 */
UCLASS(Blueprintable)
class SCHOLA_API URayCastObserver : public UBoxObserver
{
	GENERATED_BODY()

	static constexpr const float kLineGirth = 1.f;
	static constexpr const float kSphereRadius = 16.f;

public:
	/** The baseline length of each ray. Will be adjusted by the Scale component of RayTransform */
	UPROPERTY(EditAnywhere, Category = "Sensor properties", meta = (ClampMin = "1"))
	float RayLength = 4096.f;

	/** The collision channel to use for the raycast. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties|Trace Options")
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	/** Does this sensor draw debug lines/sphere's during runtime */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	bool bDrawDebugLines = false;

	/** Should the sensor trace against complex collision. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties|Trace Options")
	bool bTraceComplex = false;

	/** The number of rays to fire. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties", meta = (ClampMin = "1"))
	int32 NumRays = 2;

	/** The angle between the first and last ray. Special case of 360 degrees. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties", meta = (ClampMin = "0", ClampMax = "360"))
	float RayDegrees = 90.0f;

	/** Actor Tags that are checked on raycast collision. Included in Observations as 1-hot vector. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	TArray<FName> TrackedTags;

	/** A transform that is applied to the generated ray trajectories before firing them. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	FTransform RayStartTransform;

	/** A transform that is applied to end points of the generated ray trajectories before firing them. */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	FVector RayEndOffset;

	/** Debug colour for ray hit */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	FColor DebugHitColor = FColor::Green;

	/** Debug colour for ray miss */
	UPROPERTY(EditAnywhere, Category = "Sensor properties")
	FColor DebugMissColor = FColor::Red;

	FBoxSpace GetObservationSpace() const;

	/**
	 * @brief Generate the endpoints of the rays to be cast.
	 * @param[in] InNumRays The number of rays to generate.
	 * @param[in] InRayDegrees The angle between the first and last ray.
	 * @param[in] InBaseEnd The base endpoint of the rays.
	 * @param[in] InStart The start point of the rays.
	 * @param[in] InBaseTransform A transform to apply to the generated endpoints.
	 * @return An array of endpoints for the rays.
	 */
	TArray<FVector> GenerateRayEndpoints(int32 InNumRays, float InRayDegrees, FVector InBaseEnd, FVector InStart, FTransform InBaseTransform, FVector InEndOffset);

	/**
	 * @brief Add Empty Tags to the ray for ease of adding in one hot encodings
	 * @param[out] OutObservations The observations to append the results to.
	 */
	void AppendEmptyTags(FBoxPoint& OutObservations);

	/**
	 * @brief Helper function for appending the data based on a ray trace that hit nothing.
	 * @param[out] OutObservations The observations to append the results to.
	 * @param[in] InStart The start point of the ray.
	 * @param[in] InEnd The end point of the ray.
	 */
	void HandleRayMiss(FBoxPoint& OutObservations, FVector& InStart, FVector& InEnd);

	/**
	 * @brief Handle a SuccesfulRayTrace.
	 * @param[in] InHitResult The result of the ray trace.
	 * @param[out] OutObservations The observations to append the results to.
	 * @param[in] InStart The start point of the ray.
	 */
	void HandleRayHit(FHitResult& InHitResult, FBoxPoint& OutObservations, FVector& InStart);

	/**
	 * @brief Collect observations about the environment state.
	 * @param[out] OutObservations A BoxPoint that will be updated with the outputs of this sensor.
	 */
	virtual void CollectObservations(FBoxPoint& OutObservations) override;

#if WITH_EDITORONLY_DATA
	/** Should we draw debug lines */
	UPROPERTY()
	bool bDebugLinesEnabled = false;
#endif

#if WITH_EDITOR
	void DrawDebugLines();

	UFUNCTION(CallInEditor, Category = "Sensor Utilities")
	void ToggleDebugLines();
#endif
};
