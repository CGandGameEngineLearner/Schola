// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "Common/CommonInterfaces.h"
#include "Containers/Array.h"
#include "Containers/SortedMap.h"
#include "NNE.h"
#include "NNERuntimeCPU.h"
#include "NNERuntimeGPU.h"
#include "Points.generated.h"


class PointVisitor;
class ConstPointVisitor;
/** 
 * @brief A point is a data point that can be passed to a model for inference. This base class just provides support for the visitor pattern
*/
USTRUCT()
struct SCHOLA_API FPoint
{
	GENERATED_BODY()

	//Array View referencing other memory somewhere.
	//This should store data exactly as it would be passed to Inference
	
	virtual void Accept(PointVisitor& Visitor) PURE_VIRTUAL(FPoint::Accept, return; );

	virtual void Accept(ConstPointVisitor& Visitor) const PURE_VIRTUAL(FPoint::Accept, return; );

	virtual void Reset() PURE_VIRTUAL(FPoint::Reset, return; );

	virtual ~FPoint() = default;
};
/**
 * @brief A base class for objects that can operate on any point. This is used to implement the visitor pattern
 */
class PointVisitor
{
public:
	virtual void Visit(FBinaryPoint& Point) = 0;

	virtual void Visit(FDiscretePoint& Point) = 0;

	virtual void Visit(FBoxPoint& Point) = 0;

};
/**
 * @brief A base class for objects that can operate on any const point. This is used to implement the visitor pattern
 */
class ConstPointVisitor
{
public:
	virtual void Visit(const FBinaryPoint& Point) = 0;

	virtual void Visit(const FDiscretePoint& Point) = 0;

	virtual void Visit(const FBoxPoint& Point) = 0;

};

/**
 * @brief A point in a box(continuous) space. Conceptually a floating point vector
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FBoxPoint : public FPoint
{
	GENERATED_BODY()

	/** the values of this point */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TArray<float> Values;

	/**
	 * @brief Construct an empty BoxPoint
	 */
	FBoxPoint()
	{

	}

	/**
	 * @brief Construct a BoxPoint from a raw array of floats
	 * @param[in] Data The raw array of floats, as a const ptr
	 * @param[in] Num The size of the array
	 */
	FBoxPoint(const float* Data, int Num)
		: Values(Data,Num)
	{
	}

	/**
	 * @brief Get the value of the BoxPoint at the given index or dimension
	 * @param Index The dimension to get the value at
	 * @return the value at the given index or dimension
	 */
	float operator[](int Index) const
	{
		return this->Values[Index];
	}

	virtual ~FBoxPoint()
	{

	}

	/**
	 * @brief Add a value to the BoxPoint. Adds a new dimension to the point
	 * @param[in] Value The value to add
	 */
	void Add(float Value)
	{
		this->Values.Add(Value);
	}
	/** 
	 * @brief Reset the values of the BoxPoint. Clears the current values
	 * @note This is doesn't reset the size of the array so subsequent calls to Add will not reallocate memory
	 */
	void Reset() override
	{
		this->Values.Reset(this->Values.Num());
	};

	void Accept(PointVisitor& Visitor) override;

	void Accept(ConstPointVisitor& Visitor) const override;
};

USTRUCT(BlueprintType)
struct SCHOLA_API FBinaryPoint : public FPoint
{
	GENERATED_BODY()
	/** the values of this point */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TArray<bool> Values;
	/**
	 * @brief Construct an empty BinaryPoint
	 */
	FBinaryPoint()
	{

	}

	/**
	 * @brief Construct a BinaryPoint from a TArray of bools
	 * @param[in] InitialValues An Array of Bools to initialize the BinaryPoint with
	 */
	FBinaryPoint(TArray<bool>& InitialValues)
		: Values(InitialValues)
	{

	}

	/**
	 * @brief Construct a BinaryPoint from a raw array of bools
	 * @param[in] Data The raw array of bools, as a const ptr
	 * @param[in] Num The size of the array
	 */
	FBinaryPoint(const bool* Data, int Num)
		: Values(Data, Num)
	{

	}


	virtual ~FBinaryPoint(){};
	/**
	 * @brief Get the value of the BinaryPoint at the given index or dimension
	 * @param[in] Index The dimension to get the value at
	 * @return the value at the given index or dimension
	 */
	bool operator[](int Index) const
	{
		return this->Values[Index];
	}

	/**
	 * @brief Add a value to the BinaryPoint. Adds a new dimension to the point
	 * @param[in] Value The value to add
	 */
	void Add(bool Value)
	{
		this->Values.Add(Value);
	}

	/**
	 * @brief Reset the values of the BinaryPoint. Clears the current values
	 * @note This is doesn't reset the size of the array so subsequent calls to Add will not reallocate memory
	 */
	void Reset() override
	{
		this->Values.Reset(Values.Num());
	}
	
	void Accept(PointVisitor& Visitor) override;

	void Accept(ConstPointVisitor& Visitor) const override;
};

USTRUCT(BlueprintType)
struct SCHOLA_API FDiscretePoint : public FPoint
{
	GENERATED_BODY()
	/** the values of this point */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TArray<int> Values;

	/**
	 * @brief Construct an empty DiscretePoint
	 */
	FDiscretePoint()
	{

	}

	/**
	 * @brief Construct a DiscretePoint from a raw array of ints
	 * @param[in] Data The raw array of ints, as a const ptr
	 * @param[in] Num The size of the array
	 */
	FDiscretePoint(const int* Data, int Num)
		: Values(Data, Num)
	{

	}

	/**
	 * @brief Construct a DiscretePoint from a TArray of ints
	 * @param[in] InitialValues An Array of Ints to initialize the DiscretePoint with
	 */
	FDiscretePoint(TArray<int>& InitialValues)
		: Values(InitialValues)
	{

	}

	virtual ~FDiscretePoint()
	{

	}

	void Accept(PointVisitor& Visitor);

	void Accept(ConstPointVisitor& Visitor) const;

	/**
	 * @brief Get the value of the DiscretePoint at the given index or dimension
	 * @param[in] Index The dimension to get the value at
	 * @return the value at the given index or dimension
	 */
	int operator[](int Index) const
	{
		return this->Values[Index];
	}

	/** 
	 * @brief Reset the values of the DiscretePoint. Clears the current values
	 * @note This is doesn't reset the size of the array so subsequent calls to Add will not reallocate memory
	*/
	void Reset() override
	{
		this->Values.Reset(this->Values.Num());
	};

	/**
	 * @brief Add a value to the DiscretePoint. Adds a new dimension to the point
	 * @param[in] Value The value to add
	 */
	void Add(int Value)
	{
		this->Values.Add(Value);
	}
};

/** A variant of the three concrete point types */
typedef TVariant<FBoxPoint, FBinaryPoint, FDiscretePoint> TPoint;

/**
 * @brief A dictionary of points. This is used to store multiple points, indexed by an integer key
 */
USTRUCT()
struct FDictPoint
{
	GENERATED_BODY()

	/** The points in the dictionary */
	TArray<TPoint> Points;

	/**
	 * @brief Reset all the points in the dictionary
	 */
	void Reset();

	/**
	 * @brief Construct an empty dictionary of points
	 * @note We need to allocate the Buffer first before we start handing out array views which means we can't do it here
	 */
	FDictPoint()
	{

	}

	/**
	 * @brief Add a point to the dictionary, by emplacing it, and returning it to the caller for initialization
	 * @return a reference to the newly added point
	 */
	TPoint& Add()
	{
		return this->Points.Emplace_GetRef();
	}
	/**
	 * @brief Add a preallocated point to the dictionary
	 * @param[in] Point The point to add
	 */
	void Add(TPoint& Point)
	{
		this->Points.Add(Point);
	}

	/**
	 * @brief Get the point at the given Index, but const 
	 * @param Index The index of the point to get
	 * @return a reference to the point at the given Index
	 */
	TPoint& operator[](int Index)
	{
		return this->Points[Index];
	};
	/**
	 * @brief Get the point at the given Index, in a const context
	 * @param Index The index of the point to get
	 * @return a const reference to the point at the given Index
	 */
	const TPoint& operator[](int Index) const
	{
		return this->Points[Index];
	};

	void Accept(PointVisitor& Visitor)
	{
		for (TPoint& Point : this->Points)
		{
			Visit([&Visitor](auto& PointArg) { PointArg.Accept(Visitor); }, Point);
		}
	}

	void Accept(ConstPointVisitor& Visitor) const
	{
		for (TPoint Point : this->Points)
		{
			Visit([&Visitor](const auto& PointArg) { PointArg.Accept(Visitor); }, Point);
		}
	}

	void Accept(PointVisitor* Visitor)
	{
		return this->Accept(*Visitor);
	}

	void Accept(ConstPointVisitor* Visitor) const
	{
		return this->Accept(*Visitor);
	}

};

/** 
 * @brief A wrapper around the TensorBinding from NNE that allows us to use it in a device agnostic fashion
 */
struct SCHOLA_API FGenericTensorBinding
{
	void*  Data;
	uint64 SizeInBytes;

	operator const UE::NNE::FTensorBindingCPU() const
	{
		return { Data, SizeInBytes };
	}

	operator const UE::NNE::FTensorBindingGPU() const
	{
		return { Data, SizeInBytes };
	}

	FGenericTensorBinding(void* Data, uint64 SizeInBytes)
		: Data(Data), SizeInBytes(SizeInBytes)
	{

	}
};

