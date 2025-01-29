// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "../../Generated/Spaces.pb.h"
#include "Common/Points.h"
#include "Common/IValidatable.h"
#include "Containers/Array.h"
#include "Containers/SortedMap.h"
#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "Templates/Tuple.h"
#include "Containers/Union.h"
#include "Spaces.generated.h"


using Schola::BinarySpace;
using Schola::BoxSpace;
using Schola::DiscreteSpace;
using Schola::DictSpace;
using Schola::FundamentalSpace;

/** A variant over all Concrete Space types */
typedef TVariant<FBoxSpace, FDiscreteSpace, FBinarySpace> TSpace;

/**
 * @brief A class representing a space of possible observations or actions. This is a base class for all spaces.
 */
USTRUCT()
struct SCHOLA_API FSpace
{
	GENERATED_BODY()

	/**
	 * @brief Convert this space to a protobuf message
	 * @return A protobuf message representing this space
	 */
	FundamentalSpace*			   ToProtobuf() const;

	/**
	 * @brief Fill a protobuf message with the data from this space
	 * @param[in] Msg The protobuf message to fill
	 */
	virtual void				   FillProtobuf(FundamentalSpace* Msg) const PURE_VIRTUAL(FSpace::FillProtobuf, return; );

	/**
	 * @brief Get the number of dimensions in this space
	 * @return The number of dimensions in this space
	 */
	virtual int					   GetNumDimensions() const PURE_VIRTUAL(FSpace::FillProtobuf, return 0;);

	/**
	 * @brief Check if this space is empty
	 * @return True if this space is empty, false otherwise
	 */
	virtual bool				   IsEmpty() const PURE_VIRTUAL(FSpace::IsEmpty, return true;);

	/**
	 * @brief Test if an observation is in this space
	 * @param[in] Observation The observation to validate
	 * @return An enum indicating the result of the validation
	 */
	virtual ESpaceValidationResult Validate(TPoint& Observation) const PURE_VIRTUAL(FSpace::IsEmpty, return ESpaceValidationResult::NoResults;);
	
	/**
	 * @brief Get the size of the flattened representation of this space
	 * @return The size of the flattened representation of this space
	 */
	virtual int					   GetFlattenedSize() const PURE_VIRTUAL(FSpace::GetFlattenedSize, return 0;);

	/**
	 * @brief Create a TPoint from this space
	 * @return A TPoint belonging to this space, with correctly set variant type.
	 */
	virtual TPoint				   MakeTPoint() const PURE_VIRTUAL(FSpace::MakeTPoint, return TPoint(););
	/**
	 * @brief Normalize an observation in this space
	 * @param[in,out] Observation The observation to normalize
	 */
	virtual void				   NormalizeObservation(TPoint& Observation) const PURE_VIRTUAL(FSpace::NormalizeObservation, return; );
	/**
	 * @brief Unflatten an action from a buffer
	 * @param[in] Data The buffer to unflatten from
	 * @param[in] Offset The offset into the buffer to start unflattening from
	 */
	virtual TPoint				   UnflattenAction(const TArray<float>& Data, int Offset = 0) const PURE_VIRTUAL(FSpace::UnflattenAction, return TPoint(););
	/**
	 * @brief Flatten a point into a buffer
	 * @param[in,out] Buffer The buffer to flatten into
	 * @param[in] Point The point to flatten
	 */
	virtual void				   FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const PURE_VIRTUAL(FSpace::FlattenPoint, return; );
	virtual ~FSpace() = default;
	
};


/**
 * @brief A struct representing a dimension of a box(continuous) space of possible observations or actions.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FBoxSpaceDimension 
{
	GENERATED_BODY()

	/** The upper bound on this dimension */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	float High = 1.0;

	/** The lower bound on this dimension */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	float Low = -1.0;

	/** 
	 * @brief Construct a BoxSpaceDimension with default values
	 */
	FBoxSpaceDimension();

	/**
	 * @brief Construct a BoxSpaceDimension with the given bounds
	 * @param[in] Low The lower bound
	 * @param[in] High The upper bound
	 */
	FBoxSpaceDimension(float Low, float High);

	void FillProtobuf(Schola::BoxSpace::BoxSpaceDimension* Dimension) const;

	/**
	 * @brief Get a unit sized BoxSpaceDimension centered at 0.5
	 * @return A BoxSpaceDimension with bounds [0, 1]
	 */
	static inline FBoxSpaceDimension ZeroOneUnitDimension() { return FBoxSpaceDimension(0, 1); };

	/**
	 * @brief Get a unit sized BoxSpaceDimension centered at 0
	 * @return A BoxSpaceDimension with bounds [-0.5, 0.5]
	 */
	static inline FBoxSpaceDimension CenteredUnitDimension() { return FBoxSpaceDimension(-0.5, 0.5); };

	/**
	 * @brief Rescale a normalized value to be within this space
	 * @param[in] Value The value to rescale
	 * @return The rescaled value
	 */
	float RescaleValue(float Value) const;

	/**
	 * @brief Rescale from an another box space dimension to be within this space
	 * @param[in] Value The value to rescale
	 * @param[in] OldHigh The upper bound of the input space
	 * @param[in] OldLow The lower bound of the input space
	 * @return The rescaled value
	 */
	float RescaleValue(float Value, float OldHigh, float OldLow) const;

	/**
	 * @brief Normalize a value from this space to be in the range [0, 1]
	 * @param[in] Value The value to normalize
	 * @return The normalized value
	 */
	float NormalizeValue(float Value) const;
};


/**
 * @brief A struct representing a box(continuous) space of possible observations or actions.
 * @details A BoxSpace is a Cartesian product of BoxSpaceDimensions. Each dimension is a continuous space.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FBoxSpace : public FSpace
{
	GENERATED_BODY()

public:
	/** The dimensions of this BoxSpace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (TitleProperty = "[{Low}, {High}]"))
	TArray<FBoxSpaceDimension> Dimensions = TArray<FBoxSpaceDimension>();
	/**
	 * @brief Construct an empty BoxSpace
	 */
	FBoxSpace();

	/**
	 * @brief Construct a BoxSpace with the given bounds
	 * @param[in] Low An array representing the the lower bound of each dimension
	 * @param[in] High An array representing the the upper bound of each dimension
	 * @note Low and High must have the same length
	 */
	FBoxSpace(TArray<float>& Low, TArray<float>& High);

	/**
	 * @brief Construct a BoxSpace with the given bounds
	 * @param[in] Dimensions An array of BoxSpaceDimensions
	 */
	FBoxSpace(TArray<FBoxSpaceDimension>& Dimensions);

	/**
	 * @brief Copy constructor
	 * @param[in] Other The BoxSpace to copy
	 */
	void Copy(const FBoxSpace& Other);

	/**
	 * @brief Merge another BoxSpace into this one
	 * @param[in] Other The BoxSpace to merge
	 */
	void Merge(const FBoxSpace& Other);

	/**
	 * @brief Get the normalized version of this BoxSpace
	 * @return A BoxSpace with all dimensions normalized to [0, 1]
	 */
	FBoxSpace GetNormalizedObservationSpace() const;

	virtual ~FBoxSpace();

	/**
	 * @brief Fill a protobuf message with the data from this BoxSpace
	 * @param[in] Msg A ptr to the protobuf message to fill
	 */
	void FillProtobuf(BoxSpace* Msg) const;

	/**
	 * @brief Fill a protobuf message with the data from this BoxSpace
	 * @param[in] Msg A ref to the protobuf message to fill
	 */
	void FillProtobuf(BoxSpace& Msg) const;

	/**
	 * @brief Add a dimension to this BoxSpace
	 * @param[in] Low The lower bound of the dimension
	 * @param[in] High The upper bound of the dimension
	 */
	void Add(float Low, float High);

	/**
	 * @brief Add a dimension to this BoxSpace
	 * @param[in] Dimension The BoxSpaceDimension to add
	 */
	void Add(const FBoxSpaceDimension& Dimension);

	//FSpace API

	void FillProtobuf(FundamentalSpace* Msg) const override;

	int GetNumDimensions() const override;

	ESpaceValidationResult Validate(TPoint& Observation) const override;

	int GetFlattenedSize() const override;

	bool IsEmpty() const override;

	TPoint MakeTPoint() const override;

	void NormalizeObservation(TPoint& Observation) const;

	TPoint UnflattenAction(const TArray<float>& Data, int Offset = 0) const override;

	void FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const override;
};

/**
 * @brief A struct representing a Binary space (e.g. boolean vector) of possible observations or actions.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FBinarySpace : public FSpace
{
	GENERATED_BODY()
public:
	/** The number of dimensions in this BinarySpace*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	int Shape = 0;

	/**
	 * @brief Construct an empty BinarySpace
	 */
	FBinarySpace();

	/**
	 * @brief Construct a BinarySpace with the given number of dimensions
	 * @param[in] Shape The number of dimensions in this BinarySpace
	 */
	FBinarySpace(int Shape);

	/**
	 * @brief Merge another BinarySpace into this one
	 * @param[in] Other The BinarySpace to merge
	 */
	void Merge(const FBinarySpace& Other);

	/**
	 * @brief Copy constructor
	 * @param[in] Other The BinarySpace to copy
	 */
	void Copy(const FBinarySpace& Other);

	/**
	 * @brief fill a protobuf message with the data from this BinarySpace
	 * @param[in] Msg A ptr to the protobuf message to fill
	 */
	void FillProtobuf(BinarySpace* Msg) const;

	/**
	 * @brief fill a protobuf message with the data from this BinarySpace
	 * @param[in] Msg A ref to the protobuf message to fill
	 */
	void FillProtobuf(BinarySpace& Msg) const;

	// FSpace API

	void FillProtobuf(FundamentalSpace* Msg) const override;

	int GetNumDimensions() const override;

	ESpaceValidationResult Validate(TPoint& Observation) const override;

	int GetFlattenedSize() const override;

	bool IsEmpty() const override;

	TPoint MakeTPoint() const override;

	void NormalizeObservation(TPoint& Observation) const override;

	TPoint UnflattenAction(const TArray<float>& Data, int Offset = 0) const override;

	void FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const override;
};

/**
 * @brief A struct representing a Discrete space (e.g. Vector of integers) of possible observations or actions.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FDiscreteSpace : public FSpace
{
	GENERATED_BODY()
public:
	/** The maximum value on each dimension of this DiscreteSpace. The Lower bound is always 0. e.g. High=2 gives actions {0,1} */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (DisplayName = "Maximum Values"))
	TArray<int> High = TArray<int>();

	/**
	 * @brief Construct an empty DiscreteSpace
	 */
	FDiscreteSpace();

	/**
	 * @brief Copy construct a DiscreteSpace
	 * @param[in] Other The DiscreteSpace to copy
	 */
	void Copy(const FDiscreteSpace& Other);

	/**
	 * @brief Merge another DiscreteSpace into this one
	 * @param[in] Other The DiscreteSpace to merge
	 */
	void Merge(const FDiscreteSpace& Other);
	
	/**
	 * @brief Add a dimension to this DiscreteSpace
	 * @param[in] DimSize The maximum value of the dimension
	 */
	void Add(int DimSize);

	virtual ~FDiscreteSpace();

	/**
	 * @brief fill a protobuf message with the data from this DiscreteSpace
	 * @param[in] Msg A ptr to the protobuf message to fill
	 */
	void FillProtobuf(DiscreteSpace* Msg) const;

	/**
	 * @brief fill a protobuf message with the data from this DiscreteSpace
	 * @param[in] Msg A ref to the protobuf message to fill
	 */
	void FillProtobuf(DiscreteSpace& Msg) const;

	/**
	 * @brief Get the3 index of the maximum value in an Array of Values
	 * @param[in] Vector The vector to get the maximum values index from
	 * @return The index of the maximum value in the vector
	 */
	int GetMaxValue(const TArray<float>& Vector) const;

	// FSpace API

	void FillProtobuf(FundamentalSpace* Msg) const override;

	int GetNumDimensions() const override;

	ESpaceValidationResult Validate(TPoint& Observation) const override;

	int GetFlattenedSize() const override;

	bool IsEmpty() const override;

	TPoint MakeTPoint() const override;

	void NormalizeObservation(TPoint& Observation) const;

	TPoint UnflattenAction(const TArray<float>& Data, int Offset = 0) const override;

	void FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const override;

};

/**
 * @brief A struct representing the type of a space (e.g. Box, Discrete, Binary)
 */
UENUM()
enum class ESpaceType
{
	Box,
	Discrete,
	Binary
};


/**
 * @brief A struct representing a dictionary of possible observations or actions.
 */
USTRUCT()
struct SCHOLA_API FDictSpace
{
	GENERATED_BODY()
	// We store the dictionary as 2 arrays here. This is because we need to be able to look up spaces by label, but we also need to be able to iterate over them in order

	/** The labels of the spaces in this dictionary, used as keys for lookups */
	TArray<FString> Labels;
	/** The spaces in this dictionary */
	TArray<TSpace>	Spaces;

	/**
	 * @brief Construct an empty DictSpace
	 */
	FDictSpace();
	
	//Utility Methods
	/**
	 * @brief Get the number of spaces in this dictionary
	 * @return The number of spaces in this dictionary
	 */
	int						   Num();

	/**
	 * @brief Get the number of dimensions, of all spaces in this dictionary after flattening
	 * @return The number of dimensions
	 */
	int						   GetFlattenedSize() const;

	//Methods for working with Points in the Space
	/** 
	 * @brief Validate a point in this space, by checking if all of it's dimensions pass validation
	 * @param[in] PointMap The point to validate
	 * @return An enum indicating the result of the validation
	 */
	ESpaceValidationResult	   Validate(FDictPoint& PointMap) const;

	/**
	 * @brief Normalize an observation in this space, by normalizing each of it's dimensions
	 * @param[in,out] Observations The observation to normalize
	 */
	void					   NormalizeObservation(FDictPoint& Observations) const;
	void					   Reset();
	
	// Methods for adding things to the Space. Note that items are added in order here

	/**
	 * @brief Create a new empty space in place in this dictionary
	 * @param[in] Key The label of the space
	 * @return A reference to the newly added space
	 */
	TSpace& Add(const FString& Key);
	/**
	 * @brief Add a preallocated space from a reference to this dictionary
	 * @param[in] Key The label of the space
	 * @param[in] Value The BoxSpace to add
	 */
	void	Add(const FString& Key, TSpace& Value);
	/**
	 * @brief Add a BoxSpace to this dictionary from a reference
	 * @param[in] Key The label of the space
	 * @param[in] Value The BoxSpace to add
	 */
	void	Add(const FString& Key, FBoxSpace& Value);
	/**
	 * @brief Add a DiscreteSpace to this dictionary from a reference
	 * @param[in] Key The label of the space
	 * @param[in] Value The DiscreteSpace to add
	 */
	void	Add(const FString& Key, FDiscreteSpace& Value);
	/**
	 * @brief Add a BinarySpace to this dictionary from a reference
	 * @param[in] Key The label of the space
	 * @param[in] Value The BinarySpace to add
	 */
	void	Add(const FString& Key, FBinarySpace& Value);

	/**
	 * @brief Append another DictSpace to this one
	 * @param[in] Other The DictSpace to append
	 */
	void	Append(const FDictSpace& Other);

	// Protobuf Support
	/**
	 * @brief Convert this DictSpace to a protobuf message
	 * @return A protobuf message representing this DictSpace
	 */
	DictSpace* ToProtobuf() const;

	/**
	 * @brief Fill a protobuf message with the data from this DictSpace
	 * @param[in] Msg The protobuf message to fill
	 */
	void	   FillProtobuf(DictSpace* Msg) const;
	/**
	 * @brief Configure an empty DictPoint with the correct entries corresponding to this space
	 * @param[in,out] EmptyPoint The point to initialize
	 */ 
	void InitializeEmptyDictPoint(FDictPoint& EmptyPoint);

	/**
	 * @brief Create an empty DictPoint from a flattened point
	 * @param[in] FlattenedPoint The flattened point buffer to unflatten
	 * @return The unflattened point
	 */
	FDictPoint				   UnflattenPoint(TArray<float>& FlattenedPoint);

	/**
	 * @brief Create an empty Tensor Binding with correct size to hold a point from this DictSpace
	 * @param[in] Buffer The buffer that will contain the memory in the tensor binding
	 * @return The empty tensor binding
	 */
	FGenericTensorBinding CreateTensorBinding(TArray<float>& Buffer) const;

	/**
	 * @brief Create a Tensor Binding containing a point from this DictSpace
	 * @param[in,out] Buffer The buffer that will contain the memory in the tensor binding
	 * @param[in] Point The point to create a tensor from
	 * @return The tensor binding
	 */
	FGenericTensorBinding CreateTensorBinding(TArray<float>& Buffer, const FDictPoint& Point) const;

	/**
	 * @brief Get a subspace from this DictSpace, from an Index
	 * @param[in] Index The index of the subspace
	 * @return The subspace
	 */
	TSpace& operator[](int Index)
	{
		return this->Spaces[Index];
	};

	/**
	 * @brief Get a subspace from this DictSpace, from a Label
	 * @param[in] Label The label of the subspace
	 * @return The subspace
	 */
	TSpace& operator[](const FString& Label)
	{
		return this->Spaces[this->Labels.IndexOfByKey(Label)];
	};


};