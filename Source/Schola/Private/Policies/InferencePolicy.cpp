// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Policies/InferencePolicy.h"

int ConvertFromOneHot(TArray<int> OneHotVector)
{
	// OneHotVector should always contain a 1 otherwise behaviour is undefined
	int Index = -1;
	OneHotVector.Find(1, Index);
	return Index;
}

UInferencePolicy::UInferencePolicy()
{
}

TArray<FString> UInferencePolicy::GetRuntimeNames() const
{
	//we don't support RDG yet so skip it here
	//TArray < FString> ValidRuntimes = UE::NNE::GetAllRuntimeNames < INNERuntimeCPU>();
	//ValidRuntimes.Append(UE::NNE::GetAllRuntimeNames<INNERuntimeGPU>());
	return UE::NNE::GetAllRuntimeNames();
}

IRuntimeInterface* UInferencePolicy::GetRuntime(const FString& SelectedRuntimeName) const
{
	TWeakInterfacePtr<INNERuntimeCPU> CPUPtr = UE::NNE::GetRuntime<INNERuntimeCPU>(SelectedRuntimeName);
	if (CPUPtr.IsValid())
	{
		return new UCPURuntimeWrapper(CPUPtr);
	}

	TWeakInterfacePtr<INNERuntimeGPU> GPUPtr = UE::NNE::GetRuntime<INNERuntimeGPU>(SelectedRuntimeName);
	if (GPUPtr.IsValid())
	{
		return new UGPURuntimeWrapper(GPUPtr);
	}
	//Should probably never happen but
	return nullptr;
}

TFuture<FPolicyDecision*> UInferencePolicy::RequestDecision(const FDictPoint& Observations)
{
	TPromise<FPolicyDecision*>* DecisionPromisePtr = new TPromise<FPolicyDecision*>();
	// Get our future before it can potentially be cleaned up
	TFuture<FPolicyDecision*> FutureDecision = DecisionPromisePtr->GetFuture();
	

	if (!this->bNetworkLoaded)
	{
		DecisionPromisePtr->EmplaceValue(FPolicyDecision::PolicyError());
		delete DecisionPromisePtr;
	}
	else
	{
		AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, Observations, DecisionPromisePtr]() {
			FPolicyDecision* Decision = new FPolicyDecision(EDecisionType::ACTION);

			TArray<FGenericTensorBinding> InputBindings;
			TArray<FGenericTensorBinding> OutputBindings;

			InputBindings.Add(this->ObservationSpaceDefn.CreateTensorBinding(this->ObservationBuffer, Observations));
			OutputBindings.Add(this->ActionSpaceDefn.CreateTensorBinding(this->ActionBuffer));
			
			if ((int)this->ModelInstance->RunSync(InputBindings, OutputBindings) != 0)
			{
				DecisionPromisePtr->EmplaceValue(FPolicyDecision::PolicyError());
				UE_LOG(LogSchola, Error, TEXT("Failed to run the model"));
			}
			else
			{
				// Copy 2 and 3, maybe this does a move but I'm not sure
				Decision->Action.Values = this->ActionSpaceDefn.UnflattenPoint(this->ActionBuffer);

				// TODO cut down on the number of copy operations
				//  acann, ~11/1/2023
				DecisionPromisePtr->EmplaceValue(Decision);
			}
			
			delete DecisionPromisePtr;
		});
	}
	return FutureDecision;
}

void UInferencePolicy::Init(const FInteractionDefinition& PolicyDefinition)
{
	Step = 0;
	ActionSpaceDefn = PolicyDefinition.ActionSpaceDefn;
	ObservationSpaceDefn = PolicyDefinition.ObsSpaceDefn;
	this->ActionBuffer.SetNumZeroed(ActionSpaceDefn.GetFlattenedSize());
	this->ObservationBuffer.SetNumZeroed( ObservationSpaceDefn.GetFlattenedSize());
	
	if (ModelData)
	{
		TUniquePtr<IRuntimeInterface> Runtime = TUniquePtr<IRuntimeInterface>(this->GetRuntime(this->RuntimeName));
		if (Runtime.IsValid())
		{
			TUniquePtr<IModelInterface> TempModelPtr = Runtime->CreateModel(ModelData);
			if (TempModelPtr.IsValid())
			{
				ModelInstance = TSharedPtr<IModelInstanceInterface>(TempModelPtr->CreateModelInstance().Release());
				if (ModelInstance.IsValid())
				{
					ModelInstance->SetInputTensorShapes({ UE::NNE::FTensorShape::MakeFromSymbolic(ModelInstance->GetInputTensorDescs()[0].GetShape()) });
					bNetworkLoaded = true;

				}
				else
				{
					UE_LOG(LogSchola, Error, TEXT("Failed to create the model instance"));
					bNetworkLoaded = false;
				}
			}
			else
			{
				UE_LOG(LogSchola, Warning, TEXT("Failed to Create the Model"));
				// Invalid Runtime
				bNetworkLoaded = false;
			}
		}
		else
		{
			UE_LOG(LogSchola, Error, TEXT("Cannot find runtime %s, please enable the corresponding plugin"), *this->RuntimeName);
			// Invalid Runtime
			bNetworkLoaded = false;
		}
	}
	else
	{
		UE_LOG(LogSchola, Warning, TEXT("Failed to Create Network Due to Invalid Model Data"));
		// Invalid Model Data
		bNetworkLoaded = false;
	}
}