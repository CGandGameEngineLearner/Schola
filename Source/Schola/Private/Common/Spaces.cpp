// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Common/Spaces.h"

FundamentalSpace* FSpace::ToProtobuf() const
{
	FundamentalSpace* Msg = new FundamentalSpace();
	this->FillProtobuf(Msg);
	return Msg;
}

FBoxSpaceDimension::FBoxSpaceDimension()
{
}

FBoxSpaceDimension::FBoxSpaceDimension(float Low, float High)
	: High(High), Low(Low)
{
}

void FBoxSpaceDimension::FillProtobuf(BoxSpace::BoxSpaceDimension* Dimension) const
{
	Dimension->set_high(High);
	Dimension->set_low(Low);
}


float FBoxSpaceDimension::RescaleValue(float NormalizedValue) const
{
	return (NormalizedValue * (this->High - this->Low)) + this->Low;
}

float FBoxSpaceDimension::NormalizeValue(float Value) const
{
	// Convert a value from the range of this dimension to [0,1]
	return (Value - this->Low) / (this->High - this->Low);
}

float FBoxSpaceDimension::RescaleValue(float Value, float OldHigh, float OldLow) const
{
	// Normalize the value to be between [0,1] based on it's previous range
	float NormalizedValue = (Value - OldLow) / (OldHigh - OldLow);

	// Now blow it back up to the range of this dimension
	return this->RescaleValue(NormalizedValue);
}

FBoxSpace::FBoxSpace()
{
}

FBoxSpace::FBoxSpace(TArray<float>& Low, TArray<float>& High)
{
	for (int i = 0; i < Low.Num() && i < High.Num(); i++)
	{
		this->Dimensions.Add(FBoxSpaceDimension(Low[i], High[i]));
	}
}

FBoxSpace::FBoxSpace(TArray<FBoxSpaceDimension>& Dimensions)
{
	for (FBoxSpaceDimension& Dimension : Dimensions)
	{
		this->Dimensions.Add(FBoxSpaceDimension(Dimension));
	}
}

void FBoxSpace::Copy(const FBoxSpace& Other)
{
	this->Dimensions = TArray<FBoxSpaceDimension>(Other.Dimensions);
}

void FBoxSpace::Merge(const FBoxSpace& Other)
{
	this->Dimensions.Append(Other.Dimensions);
}

ESpaceValidationResult FBoxSpace::Validate(TPoint& Observation) const
{
	if (!Observation.IsType<FBoxPoint>())
	{
		return ESpaceValidationResult::WrongDataType;
	}
	FBoxPoint& TypedObservation = Observation.Get<FBoxPoint>();

	// Is the shape right?
	if (Dimensions.Num() != TypedObservation.Values.Num())
	{
		return ESpaceValidationResult::WrongDimensions;
	}
	

	// Are all our values in our bounding box?
	for (int i = 0; i < Dimensions.Num(); i++)
	{
		if (TypedObservation.Values[i] > Dimensions[i].High || TypedObservation.Values[i] < Dimensions[i].Low)
		{
			return ESpaceValidationResult::OutOfBounds;
		}
	}

	return ESpaceValidationResult::Success;
}

void FBoxSpace::NormalizeObservation(TPoint& Observation) const
{
	FBoxPoint& TypedObservation = Observation.Get<FBoxPoint>();
	// Use bounding box to normalize the observations, we can safely do so because it has already been validated
	for (int i = 0; i < Dimensions.Num(); i++)
	{
		TypedObservation.Values[i] = Dimensions[i].NormalizeValue(TypedObservation.Values[i]);
	}
}

FBoxSpace FBoxSpace::GetNormalizedObservationSpace() const
{
	FBoxSpace OutBoxSpace;

	// Set extent to be between 0 and 1 for normalized observations.
	for (int i = 0; i < Dimensions.Num(); i++)
	{
		OutBoxSpace.Add(FBoxSpaceDimension::ZeroOneUnitDimension());
	}
	return OutBoxSpace;
}

FBoxSpace::~FBoxSpace()
{
	this->Dimensions.Empty();
}

int FBoxSpace::GetNumDimensions() const
{
	return Dimensions.Num();
}

void FBoxSpace::FillProtobuf(BoxSpace* Msg) const
{
	this->FillProtobuf(*Msg);
}

void FBoxSpace::FillProtobuf(BoxSpace& Msg) const
{
	for (const FBoxSpaceDimension& Dimension : this->Dimensions)
	{
		Dimension.FillProtobuf(Msg.add_dimensions());
	}
}

void FBoxSpace::FillProtobuf(FundamentalSpace* Msg) const
{
	this->FillProtobuf(Msg->mutable_box_space());
}


TPoint FBoxSpace::UnflattenAction(const TArray<float>& Data, int Offset) const
{
	TPoint OutPoint = TPoint(TInPlaceType<FBoxPoint>(), (const float*)Data.GetData() + Offset, Dimensions.Num());
	return OutPoint;
}

void FBoxSpace::FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const
{
	assert(Buffer.Num() == this->GetFlattenedSize());
	TArray<float> Arr = Point.Get<FBoxPoint>().Values;
	for (int i = 0; i < Arr.Num();i++)
	{
		Buffer[i] = Arr[i];
	}
}

int FBoxSpace::GetFlattenedSize() const
{
	return Dimensions.Num();
}

void FBoxSpace::Add(float Low, float High)
{
	this->Dimensions.Add(FBoxSpaceDimension(Low, High));
}

void FBoxSpace::Add(const FBoxSpaceDimension& Dimension)
{
	this->Dimensions.Add(Dimension);
}

bool FBoxSpace::IsEmpty() const
{
	return this->GetNumDimensions() == 0;
}

TPoint FBoxSpace::MakeTPoint() const
{
	return TPoint(TInPlaceType<FBoxPoint>());
}

FBinarySpace::FBinarySpace()
{
}

FBinarySpace::FBinarySpace(int Shape)
{
	this->Shape = Shape;
}

void FBinarySpace::Merge(const FBinarySpace& Other)
{
	this->Shape += Other.Shape;
}

void FBinarySpace::Copy(const FBinarySpace& Other)
{
	this->Shape = Other.Shape;
}

void FBinarySpace::FillProtobuf(FundamentalSpace* Msg) const
{

	this->FillProtobuf(Msg->mutable_binary_space());
}

int FBinarySpace::GetNumDimensions() const
{
	return Shape;
}

ESpaceValidationResult FBinarySpace::Validate(TPoint& Observation) const
{

	// Is the shape right?
	if (!Observation.IsType<FBinaryPoint>())
	{
		return ESpaceValidationResult::WrongDataType;
	}
	FBinaryPoint& TypedObservation = Observation.Get<FBinaryPoint>();


	if (Shape != TypedObservation.Values.Num())
	{
		return ESpaceValidationResult::WrongDimensions;
	}
	else
	{
		return ESpaceValidationResult::Success;
	}
}

void FBinarySpace::NormalizeObservation(TPoint& Observation) const
{
	// Binary observations are already normalized
}

void FBinarySpace::FillProtobuf(BinarySpace* Msg) const
{
	this->FillProtobuf(*Msg);
}

void FBinarySpace::FillProtobuf(BinarySpace& Msg) const
{
	Msg.set_shape(Shape);
}

TPoint FBinarySpace::UnflattenAction(const TArray<float>& Data, int Offset) const
{
	TArray<bool> BoolArray;
	for (int i = 0; i < GetNumDimensions(); i++)
	{
		BoolArray.Add(static_cast<bool>(Data[i + Offset]));
	}
	return TPoint(TInPlaceType<FBinaryPoint>(), BoolArray);
}

void FBinarySpace::FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const
{
	assert(Buffer.Num() == this->GetFlattenedSize());
	TArray<bool> Arr = Point.Get<FBinaryPoint>().Values;
	for (int i = 0; i < Arr.Num(); i++)
	{
		Buffer[i] = Arr[i];
	}
}

int FBinarySpace::GetFlattenedSize() const
{
	return this->Shape;
}

bool FBinarySpace::IsEmpty() const
{
	return this->Shape == 0;
}

TPoint FBinarySpace::MakeTPoint() const
{
	return TPoint(TInPlaceType<FBinaryPoint>());
}

FDiscreteSpace::FDiscreteSpace()
{
}

void FDiscreteSpace::Copy(const FDiscreteSpace& Other)
{
	this->High = TArray<int>(Other.High);
}

void FDiscreteSpace::Merge(const FDiscreteSpace& Other)
{
	this->High.Append(Other.High);
}

void FDiscreteSpace::Add(int DimSize)
{
	this->High.Add(DimSize);
}

ESpaceValidationResult FDiscreteSpace::Validate(TPoint& Observation) const
{
	// Is the shape right?
	if (!Observation.IsType<FDiscretePoint>())
	{
		return ESpaceValidationResult::WrongDataType;
	}
	FDiscretePoint& TypedObservation = Observation.Get<FDiscretePoint>();

	if (High.Num() != TypedObservation.Values.Num())
	{
		return ESpaceValidationResult::WrongDimensions;
	}

	// Are all our values in our bounding box?
	for (int i = 0; i < High.Num(); i++)
	{
		if (TypedObservation.Values[i] > High[i] || TypedObservation.Values[i] < 0)
		{
			return ESpaceValidationResult::OutOfBounds;
		}
	}

	return ESpaceValidationResult::Success;
}

void FDiscreteSpace::NormalizeObservation(TPoint& Observation) const
{
	// Discrete observations are already normalized
}

void FDiscreteSpace::FillProtobuf(FundamentalSpace* Msg) const
{
	this->FillProtobuf(Msg->mutable_discrete_space());
}

int FDiscreteSpace::GetNumDimensions() const
{
	return High.Num();
}

FDiscreteSpace::~FDiscreteSpace()
{
	this->High.Empty();
}

void FDiscreteSpace::FillProtobuf(DiscreteSpace* Msg) const
{
	this->FillProtobuf(*Msg);
}

void FDiscreteSpace::FillProtobuf(DiscreteSpace& Msg) const
{
	for (const int& HighValue : High)
	{
		Msg.add_high(HighValue);
	}
}

int FDiscreteSpace::GetMaxValue(const TArray<float>& Vector) const
{
	float CurrMax = Vector[0];
	int	  Index = 0;
	int	  CurrIndex = 0;
	for (const float& Value : Vector)
	{
		if (Value > CurrMax)
		{
			CurrMax = Value;
			Index = CurrIndex;
		}
		CurrIndex += 1;
	}
	return Index;
}

TPoint FDiscreteSpace::UnflattenAction(const TArray<float>& Data, int Offset) const
{
	
	TPoint Point = this->MakeTPoint();
	FDiscretePoint& TypedPoint = Point.Get<FDiscretePoint>();
	int			   OutputValue = 0;
	int			   CurrIndex = 0;
	int			   dim = GetNumDimensions();
	//TODO remove the copy from this function
	for (int i = 0; i < dim; i++)
	{
		TArray<float> BranchArray = {};
		int			  BranchHigh = High[i];
		for (int j = CurrIndex; j < BranchHigh + CurrIndex; j++)
		{
			BranchArray.Add(Data[j + Offset]);
		}
		OutputValue = GetMaxValue(BranchArray);
		TypedPoint.Values.Add(OutputValue);
		CurrIndex += High[i];
	}

	return Point;
}

void FDiscreteSpace::FlattenPoint(TArrayView<float> Buffer, const TPoint& Point) const
{
	//WE assume that the buffer is zeroed out
	assert(Buffer.Num() == this->GetFlattenedSize());
	TArray<int> Arr = Point.Get<FDiscretePoint>().Values;
	int			BranchStart = 0;
	for (int i = 0; i < this->High.Num(); i++)
	{
		Buffer[Arr[i] + BranchStart] = 1;
		BranchStart += this->High[i];
	}
}

int FDiscreteSpace::GetFlattenedSize() const
{
	int Size = 0;
	for (const int& HighValue : High)
	{
		Size += HighValue;
	}
	return Size;
}

bool FDiscreteSpace::IsEmpty() const
{
	return this->GetNumDimensions() == 0;
}

TPoint FDiscreteSpace::MakeTPoint() const
{
	return TPoint(TInPlaceType<FDiscretePoint>());
}

FDictSpace::FDictSpace()
{
}


int FDictSpace::Num()
{
	return 0;
}


int FDictSpace::GetFlattenedSize() const
{	
	int Size = 0;
	for (const TSpace& Space : this->Spaces)
	{
		Size += Visit([](auto _Space) { return _Space.GetFlattenedSize(); }, Space);
	}

	return Size;
}

ESpaceValidationResult FDictSpace::Validate(FDictPoint& PointMap) const
{
	ESpaceValidationResult Result = ESpaceValidationResult::NoResults;

	for (int i =0; i < this->Spaces.Num(); i++)
	{
		const TSpace& Space = this->Spaces[i];
		TPoint& Point = PointMap[i];
		ESpaceValidationResult CurrentResult = Visit([&Point](auto& TypedSpace) { return TypedSpace.Validate(Point); }, Space);
		switch (CurrentResult)
		{
			case ESpaceValidationResult::NoResults:
				break;
			case ESpaceValidationResult::Success:
				Result = CurrentResult;
				break;
			default:
				Result = CurrentResult;
				return Result;
		}

	}

	return Result;
}

void FDictSpace::NormalizeObservation(FDictPoint& Observations) const
{
	for (int i = 0; i < this->Spaces.Num(); i++)
	{
		TPoint& CurrentObs = Observations[i];
		Visit([&CurrentObs](auto& TypedSpace) {TypedSpace.NormalizeObservation(CurrentObs); }, this->Spaces[i]);
	}
}

void FDictSpace::Reset()
{
	this->Labels.Empty();
	this->Spaces.Empty();
}

DictSpace* FDictSpace::ToProtobuf() const
{
	DictSpace* RetVal = new DictSpace();
	FillProtobuf(RetVal);
	return RetVal;
}

void FDictSpace::FillProtobuf(DictSpace* Msg) const
{
	for (int i =0; i < this->Spaces.Num();i++)
	{
		Msg->add_labels(TCHAR_TO_UTF8(*this->Labels[i]));
		FundamentalSpace* SpaceMsg = Msg->add_values();
		Visit([SpaceMsg](auto& TypedSpace) { TypedSpace.FillProtobuf(SpaceMsg); }, this->Spaces[i]);
	}
}

void FDictSpace::InitializeEmptyDictPoint(FDictPoint& EmptyPoint)
{	
	EmptyPoint.Points.Empty();
	for (TSpace& Space : this->Spaces)
	{
		EmptyPoint.Points.Add((Visit([](auto& TypedSpace) { return TypedSpace.MakeTPoint(); }, Space)));
	}
}

FDictPoint FDictSpace::UnflattenPoint(TArray<float>& FlattenedPoint)
{
	int StartIndex = 0;
	FDictPoint Output = FDictPoint();
	for (const TSpace& Space : this->Spaces)
	{
		int Size = Visit([](auto& TypedSpace) { return TypedSpace.GetFlattenedSize(); }, Space);
		TPoint Point = Visit([&FlattenedPoint, StartIndex](auto& TypedSpace) { return TypedSpace.UnflattenAction(FlattenedPoint, StartIndex); }, Space);
		Output.Points.Add(Point);
		StartIndex += Size;
	}
	return Output;
}

FGenericTensorBinding FDictSpace::CreateTensorBinding(TArray<float>& EmptyBuffer) const
{
	int Size = this->GetFlattenedSize();
	return { EmptyBuffer.GetData(), Size * sizeof(float) };
}

FGenericTensorBinding FDictSpace::CreateTensorBinding(TArray<float>& Buffer, const FDictPoint& DictPoint) const
{
	int Size = this->GetFlattenedSize();
	int Offset = 0;

	for (int i = 0; i < this->Spaces.Num(); i++)
	{
		const TSpace& Space = this->Spaces[i];
		const TPoint&		  Point = DictPoint[i];
		int			  Count = Visit([](auto& TypedSpace) { return TypedSpace.GetFlattenedSize(); }, Space);
		Visit([&Point, &Buffer, Offset, Count](auto& TypedSpace) { TypedSpace.FlattenPoint(MakeArrayView(Buffer.GetData() + Offset, Count), Point); }, Space);
		Offset += Count;
	}
	return {Buffer.GetData(), Size * sizeof(float) };
}

TSpace& FDictSpace::Add(const FString& Label)
{
	this->Labels.Add(Label);
	return this->Spaces.Emplace_GetRef();
}
