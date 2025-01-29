// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observers/AbstractObservers.h"
#include "Common/Spaces.h"
#include "Common/Points.h"
#include "DebugObservers.generated.h"


/**
 * @brief A Debug version of BoxObserver that generates random observations.
 */
UCLASS(Blueprintable)
class SCHOLA_API UDebugBoxObserver : public UBoxObserver
{
	GENERATED_BODY()

public:
	UDebugBoxObserver(){};

	/** The ObservationSpace bounding the outputs of this sensor. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBoxSpace ObservationSpace = FBoxSpace();

	FBoxSpace GetObservationSpace() const;

	void CollectObservations(FBoxPoint& OutObservations);
};

/**
 * @brief A Debug Version of BinaryObserver that generates random observations
 */
UCLASS(Blueprintable)
class SCHOLA_API UDebugBinaryObserver : public UBinaryObserver
{
	GENERATED_BODY()

public:
	UDebugBinaryObserver(){};

	/** The ObservationSpace bounding the outputs of this sensor. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBinarySpace ObservationSpace;
	
	FBinarySpace GetObservationSpace() const;

	void CollectObservations(FBinaryPoint& OutObservations);
};

/**
 * @brief A Debug Version of DiscreteObserver that generates random observations
 */
UCLASS(Blueprintable)
class SCHOLA_API UDebugDiscreteObserver : public UDiscreteObserver
{
	GENERATED_BODY()
public:
	UDebugDiscreteObserver(){};

	/** The ObservationSpace bounding the outputs of this sensor. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDiscreteSpace ObservationSpace;

	FDiscreteSpace GetObservationSpace() const;

	void CollectObservations(FDiscretePoint& OutObservations);
};
