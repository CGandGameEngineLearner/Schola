
#include "ScholaEditor.h"

#define LOCTEXT_NAMESPACE "ScholaEditor"

void FScholaEditorModule::StartupModule()
{
	// Agent
	FKismetEditorUtilities::FOnBlueprintCreated AgentCallback;
	RegisterDefaultFunction(ABlueprintTrainer, AgentCallback, ComputeReward);
	RegisterDefaultFunction(ABlueprintTrainer, AgentCallback, ComputeStatus);
	RegisterDefaultFunction(ABlueprintTrainer, AgentCallback, GetInfo);

	// Environment
	FKismetEditorUtilities::FOnBlueprintCreated EnvironmentCallback;
	RegisterDefaultFunction(ABlueprintScholaEnvironment, EnvironmentCallback, RegisterAgents);
	RegisterDefaultEvent(ABlueprintScholaEnvironment, ResetEnvironment);
	RegisterDefaultEvent(ABlueprintScholaEnvironment, InitializeEnvironment);
	RegisterDefaultEvent(ABlueprintScholaEnvironment, SetEnvironmentOptions);
	RegisterDefaultEvent(ABlueprintScholaEnvironment, SeedEnvironment);

	// Observers
	FKismetEditorUtilities::FOnBlueprintCreated DiscreteObserverCallback;
	RegisterDefaultFunction(UBlueprintDiscreteObserver, DiscreteObserverCallback, GetObservationSpace);
	RegisterDefaultFunction(UBlueprintDiscreteObserver, DiscreteObserverCallback, CollectObservations);

	FKismetEditorUtilities::FOnBlueprintCreated BinaryObserverCallback;
	RegisterDefaultFunction(UBlueprintBinaryObserver, BinaryObserverCallback, GetObservationSpace);
	RegisterDefaultFunction(UBlueprintBinaryObserver, BinaryObserverCallback, CollectObservations);

	FKismetEditorUtilities::FOnBlueprintCreated BoxObserverCallback;
	RegisterDefaultFunction(UBlueprintBoxObserver, BoxObserverCallback, GetObservationSpace);
	RegisterDefaultFunction(UBlueprintBoxObserver, BoxObserverCallback, CollectObservations);


	//Actuators
	FKismetEditorUtilities::FOnBlueprintCreated DiscreteActuatorCallback;
	RegisterDefaultFunction(UBlueprintDiscreteActuator, DiscreteActuatorCallback, GetActionSpace);
	RegisterDefaultFunction(UBlueprintDiscreteActuator, DiscreteActuatorCallback, TakeAction);

	FKismetEditorUtilities::FOnBlueprintCreated BinaryActuatorCallback;
	RegisterDefaultFunction(UBlueprintBinaryActuator, BinaryActuatorCallback, GetActionSpace);
	RegisterDefaultFunction(UBlueprintBinaryActuator, BinaryActuatorCallback, TakeAction);

	FKismetEditorUtilities::FOnBlueprintCreated BoxActuatorCallback;
	RegisterDefaultFunction(UBlueprintBoxActuator, BoxActuatorCallback, GetActionSpace);
	RegisterDefaultFunction(UBlueprintBoxActuator, BoxActuatorCallback, TakeAction);

}

void FScholaEditorModule::ShutdownModule()
{
	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(this);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FScholaEditorModule, ScholaEditor);