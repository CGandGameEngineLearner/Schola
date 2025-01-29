// Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Common/AbstractInteractor.h"

AActor* UAbstractInteractor::TryGetOwner() const
{
	//Four possible locations
	// 1. In a component on the Agent
	// 2. In a component on the Trainer/Controller
	// 3. In the controller directly
	// 4. In the Agent Directly

	UObject* Outer = this->GetOuter();
	if (Outer)
	{
		UActorComponent* Comp = Cast<UActorComponent>(Outer);
		AController*	 Controller = Cast<AController>(Outer);
		
		if (Comp)
		{	
			// In a Component
			AActor* CompOwner = Comp->GetOwner();
			Controller = Cast<AController>(CompOwner);
			
			if (Controller)
			{
				// In a Component in the Trainer
				return Controller->GetPawn();
			}
			else
			{
				return CompOwner;
			}
		} 
		else if (Controller) 
		{
			// Directly in the Trainer
			return Controller->GetPawn();
		} 
		else 
		{ 
			//In the Pawn
			return Cast<AActor>(Outer);
		}
	}
	else
	{
		return nullptr;
	}
}

UObject* UAbstractInteractor::GetLocation() const
{
	UObject* Outer = this->GetOuter();
	if (Outer)
	{
		UActorComponent* Comp = Cast<UActorComponent>(Outer);
		AController*	 Controller = Cast<AController>(Outer);

		if (Comp)
		{
			return Comp->GetOwner(); //Return Either the Controller, if that is where this comp is, or the Pawn
		}
		else if (Controller)
		{
			return Controller->GetOuter(); // Return the Pawn or other entity owning this Controller
		}
		else
		{
			return Outer;
		}
	}
	else
	{
		return nullptr;
	}
}

FString UAbstractInteractor::GetLabel() const
{
	UObject* Outer = this->GetOuter();
	if (Outer)
	{
		UActorComponent* Comp = Cast<UActorComponent>(Outer);
		AController*	 Controller = Cast<AController>(Outer);

		if (Comp)
		{
			return Comp->GetName(); // Return Either the Controller, if that is where this comp is, or the Pawn
		}
		else 
		{
			return Outer->GetClass()->GetName() + FString("_") + this->GetName(); // Return the Pawn or other entity owning this Controller
		}
	}
	else
	{
		return FString("None_") + this->GetName();
	}
}

FString UAbstractInteractor::GetId(int IntId) const
{
	//Breaks if the number of interactors is greater than 99999 which is totally okay.
	return FString("").Appendf(TEXT("%05d"), IntId).Append("_").Append(this->GetLabel());
	
}