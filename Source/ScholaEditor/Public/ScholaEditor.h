#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "BlueprintEditorLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditor.h"
#include "UnrealEd.h"


#include "Training/AbstractTrainer.h"
#include "Environment/AbstractEnvironment.h"
#include "Observers/AbstractObservers.h"
#include "Common/UtilityFunctions.h"


class FScholaEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

};