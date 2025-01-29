// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Policies/AbstractPolicy.h"
#include "NNE.h"
#include "NNERuntimeCPU.h"
#include "NNEModelData.h"
#include "Common/InteractionDefinition.h"
#include "Agent/AgentAction.h"
#include "Common/LogSchola.h"
#include "Common/Spaces.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include <type_traits>
#include "NNEStatus.h"
#include "InferencePolicy.generated.h"

/**
 * @brief Generic Interface for a model instance wrapping NNE ModelInstaces targetted at different devices
 */
class IModelInstanceInterface
{
public:
	virtual ~IModelInstanceInterface() = default;
	virtual TConstArrayView<UE::NNE::FTensorDesc> GetInputTensorDescs() = 0;
	virtual UE::NNE::EResultStatus				  SetInputTensorShapes(TConstArrayView<UE::NNE::FTensorShape> InInputShapes) = 0;
	virtual UE::NNE::EResultStatus				  RunSync(TConstArrayView<FGenericTensorBinding> InInputBindings, TConstArrayView<FGenericTensorBinding> InOutputBinding) = 0;
};


/**
 * @brief Generic Interface for a model wrapping NNE Models targetted at different devices
 * @tparam T The type of the model instance to be wrapped
 */
template <class T>
class UModelInstanceWrapper : public IModelInstanceInterface
{
private:
	TSharedPtr<T> WrappedModel;

	template <typename BindingType>
	struct FBindingTypeExtractor;

	template <typename BindingType>
	struct FBindingTypeExtractor<UE::NNE::EResultStatus (T::*)(TConstArrayView<BindingType>, TConstArrayView<BindingType>)>
	{
		using BoundType = BindingType;
	};
public:
	UModelInstanceWrapper(TSharedPtr<T> ModelPtr)
		: WrappedModel(ModelPtr)
	{
	}

	using BindingType = FBindingTypeExtractor<decltype(&T::RunSync)>::BoundType;

	TConstArrayView<UE::NNE::FTensorDesc> GetInputTensorDescs()
	{
		return WrappedModel->GetInputTensorDescs();
	};

	UE::NNE::EResultStatus SetInputTensorShapes(TConstArrayView<UE::NNE::FTensorShape> InInputShapes) override
	{
		return WrappedModel->SetInputTensorShapes(InInputShapes);
	};

	UE::NNE::EResultStatus RunSync(TConstArrayView<FGenericTensorBinding> InInputBindings, TConstArrayView<FGenericTensorBinding> InOutputBindings) override
	{
		return WrappedModel->RunSync(MakeArrayView((BindingType*)InInputBindings.GetData(), InInputBindings.Num()), MakeArrayView((BindingType*)InOutputBindings.GetData(), InOutputBindings.Num()));
	};
};

/**
 * @brief Generic Interface for a model wrapping NNE Models targetted at different devices
 */
class IModelInterface
{

public:
	virtual ~IModelInterface() = default;

	virtual TUniquePtr<IModelInstanceInterface> CreateModelInstance() = 0;
};

/**
 * @brief Wrapper around NNE CPU Models
 */
class UCPUModelWrapper : public IModelInterface
{
	TSharedPtr<UE::NNE::IModelCPU>		ModelPtr;
public:
	UCPUModelWrapper(TSharedPtr<UE::NNE::IModelCPU> RawModel)
		: ModelPtr(RawModel){

	};

	TUniquePtr<IModelInstanceInterface> CreateModelInstance()
	{
		return TUniquePtr<IModelInstanceInterface>(new UModelInstanceWrapper<UE::NNE::IModelInstanceCPU>(this->ModelPtr->CreateModelInstanceCPU()));
	};
};

/**
 * @brief Wrapper around NNE GPU Models
 */
class UGPUModelWrapper : public IModelInterface
{
	TSharedPtr<UE::NNE::IModelGPU> ModelPtr;
public:
	UGPUModelWrapper(TSharedPtr<UE::NNE::IModelGPU> RawModel)
		: ModelPtr(RawModel){

	};

	TUniquePtr<IModelInstanceInterface> CreateModelInstance()
	{
		return TUniquePtr<IModelInstanceInterface>(new UModelInstanceWrapper<UE::NNE::IModelInstanceGPU>(this->ModelPtr->CreateModelInstanceGPU()));
	};
};

/**
 * @brief Generic Interface for a runtime wrapping NNE Runtimes targetted at different devices
 */
class IRuntimeInterface
{
public:
	virtual ~IRuntimeInterface() = default;
	virtual TUniquePtr<IModelInterface> CreateModel(TObjectPtr<UNNEModelData> ModelData) = 0;
	virtual bool						IsValid() = 0;
};

/**
 * @brief Wrapper around NNE CPU Runtimes
 */
class UCPURuntimeWrapper : public IRuntimeInterface
{
	TWeakInterfacePtr<INNERuntimeCPU> RuntimePtr;

public:
	UCPURuntimeWrapper(TWeakInterfacePtr<INNERuntimeCPU> RawRuntime)
		: RuntimePtr(RawRuntime)
	{

	}

	bool IsValid()
	{
		return RuntimePtr.IsValid();
	}

	TUniquePtr<IModelInterface> CreateModel(TObjectPtr<UNNEModelData> ModelData)
	{
		return TUniquePtr<IModelInterface>(new UCPUModelWrapper(this->RuntimePtr->CreateModelCPU(ModelData)));
	};
};

/**
 * @brief Wrapper around NNE GPU Runtimes
 */
class UGPURuntimeWrapper : public IRuntimeInterface
{
	TWeakInterfacePtr<INNERuntimeGPU> RuntimePtr;

public:
	UGPURuntimeWrapper(TWeakInterfacePtr<INNERuntimeGPU> RawRuntime)
		: RuntimePtr(RawRuntime)
	{

	}

	bool IsValid()
	{
		return RuntimePtr.IsValid();
	}

	TUniquePtr<IModelInterface> CreateModel(TObjectPtr<UNNEModelData> ModelData)
	{
		return TUniquePtr<IModelInterface>(new UGPUModelWrapper(this->RuntimePtr->CreateModelGPU(ModelData)));
	};
};

/**
 * @brief Enum for the different types of runtimes
 */
UENUM(BlueprintType)
enum class ERuntimeType : uint8
{
	CPU,
	GPU
};

/**
 * @brief Policy that uses a trained NNE model to make decisions
 */
UCLASS(Blueprintable, EditInlineNew)
class SCHOLA_API UInferencePolicy : public UAbstractPolicy
{
	GENERATED_BODY()

public:
	UInferencePolicy();

	/** Number of inference calls to the policy */
	UPROPERTY()
	int Step = 0;

	/** The Action Space of the Agent */
	UPROPERTY()
	FDictSpace ActionSpaceDefn;

	/** The Observation Space of the Agent */
	UPROPERTY()
	FDictSpace ObservationSpaceDefn;

	/** The Model Data for the NNE Model */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNNEModelData> ModelData;

	/** The Name of the Runtime to use for Inference. If no Runtimes are available, enable NNE plugins (e.g. NNERuntimeORT) */
	UPROPERTY(EditAnywhere, meta=(GetOptions="GetRuntimeNames"))
	FString RuntimeName;

	/** 
	 * @brief Function that grabs all availabe runtime names 
	 * @return An Array of available runtime names
	 */
	UFUNCTION()
	TArray<FString> GetRuntimeNames() const;

	/**
	 * @brief Function that gets the singleton instance of the runtime with the given name
	 * @param[in] SelectedRuntimeName The name of the runtime to get
	 * @return A ptr to the runtime with the given name
	 */
	IRuntimeInterface* GetRuntime(const FString& SelectedRuntimeName) const;

	/** Variable tracking if the network loaded correctly?*/
	UPROPERTY(VisibleAnywhere)
	bool bNetworkLoaded = false;

	virtual TFuture<FPolicyDecision*> RequestDecision(const FDictPoint& Observations) override;

	void Init(const FInteractionDefinition& PolicyDefinition);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<float> ActionBuffer;

	UPROPERTY(BlueprintReadOnly,VisibleAnywhere)
	TArray<float> ObservationBuffer;

private:
	/** The instantiated model */
	TSharedPtr<IModelInstanceInterface> ModelInstance;
};