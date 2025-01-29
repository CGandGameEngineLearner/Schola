// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Common/Points.h"
#include "../Generated/Spaces.pb.h"


void FDictPoint::Reset()
{
	this->Points.Reset(this->Points.Num());
}

void FBoxPoint::Accept(ConstPointVisitor& Visitor) const
{
	Visitor.Visit(*this);
}

void FBoxPoint::Accept(PointVisitor& Visitor)
{
	Visitor.Visit(*this);
}

void FDiscretePoint::Accept(ConstPointVisitor& Visitor) const
{
	Visitor.Visit(*this);
}


void FDiscretePoint::Accept(PointVisitor& Visitor)
{
	Visitor.Visit(*this);
}

void FBinaryPoint::Accept(ConstPointVisitor& Visitor) const
{
	Visitor.Visit(*this);
}

void FBinaryPoint::Accept(PointVisitor& Visitor)
{
	Visitor.Visit(*this);
}