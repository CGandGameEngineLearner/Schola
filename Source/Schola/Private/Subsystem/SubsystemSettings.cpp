// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#include "Subsystem/SubsystemSettings.h"

inline FString WithQuotes(FString Input)
{
	return FString("\"") + Input + FString("\"");
}

UScholaManagerSubsystemSettings::UScholaManagerSubsystemSettings()
{
}

FLaunchableScript UScholaManagerSubsystemSettings::GetScript() const
{
	FLaunchableScript Script = this->ScriptSettings.GetLaunchableScript();
	FString			  TrainingArgs = this->ScriptSettings.GetTrainingArgs(this->CommunicatorSettings.Port);
	Script.AppendArgs(TrainingArgs);
	return Script;
}

void FRLlibTrainingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddIntArg(TEXT("port"), Port);
	ArgBuilder.AddIntArg(TEXT("timesteps"), Timesteps);

	this->CheckpointSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->LoggingSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->ResumeSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->NetworkArchitectureSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->ResourceSettings.GenerateTrainingArgs(Port, ArgBuilder);
}

FRLlibTrainingSettings::~FRLlibTrainingSettings()
{
}

void FSB3TrainingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	this->CheckpointSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->LoggingSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->ResumeSettings.GenerateTrainingArgs(Port, ArgBuilder);
	this->NetworkArchitectureSettings.GenerateTrainingArgs(Port, ArgBuilder);
	//Note that if the NetworkArch Args go right before the Algorithm, the Algorithm gets eaten by the Variable length argument defining network arch.

	ArgBuilder.AddIntArg(TEXT("port"), Port);
	ArgBuilder.AddIntArg(TEXT("timesteps"), Timesteps);
	ArgBuilder.AddFlag(TEXT("pbar"), bDisplayProgressBar);

	switch (this->Algorithm)
	{
		case (ETrainingAlgorithm::SAC):
			ArgBuilder.AddPositionalArgument(TEXT("SAC"));
			this->SACSettings.GenerateTrainingArgs(Port, ArgBuilder);
			break;
		default:
			ArgBuilder.AddPositionalArgument(TEXT("PPO"));
			this->PPOSettings.GenerateTrainingArgs(Port, ArgBuilder);
			break;
	}
}

FSB3TrainingSettings::~FSB3TrainingSettings()
{
}

void FTrainingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	;
}

FProcHandle FLaunchableScript::LaunchScript() const
{
	return FPlatformProcess::CreateProc(TEXT("cmd.exe"), *(FString("/K ") + this->ScriptURL  + FString(" ") + this->Args), false, false, false, nullptr, 0, nullptr, nullptr);
}

FString FScriptSettings::GetTrainingArgs(int Port) const
{
	FScriptArgBuilder ArgBuilder = FScriptArgBuilder();
	switch (ScriptType)
	{
		case (EScriptType::Python):
			switch (PythonScriptType)
			{
				case (EPythonScript::SB3):
					this->SB3Settings.GenerateTrainingArgs(Port, ArgBuilder);
					break;
				case (EPythonScript::RLLIB):
					this->RLlibSettings.GenerateTrainingArgs(Port, ArgBuilder);
					break;
				default:
					this->CustomPythonScriptSettings.GenerateTrainingArgs(Port, ArgBuilder);
					break;
			}
			break;

		default:
			this->CustomScriptSettings.GenerateTrainingArgs(Port, ArgBuilder);
			break;
	}
	return ArgBuilder.Build();
}

FFilePath FScriptSettings::GetScriptPath() const
{
	switch (ScriptType)
	{
		case (EScriptType::Python):
			switch (PythonScriptType)
			{
				case (EPythonScript::SB3):
					return FFilePath{ IPluginManager::Get().FindPlugin(TEXT("Schola"))->GetBaseDir() + FString("/Resources/python/schola/scripts/sb3/launch.py") };

				case (EPythonScript::RLLIB):
					return FFilePath{ IPluginManager::Get().FindPlugin(TEXT("Schola"))->GetBaseDir() + FString("/Resources/python/schola/scripts/ray/launch.py") };

				default:
					return CustomPythonScriptSettings.LaunchScript;
			}

		default:
			return CustomScriptSettings.LaunchScript;
	}
}

FLaunchableScript FScriptSettings::GetLaunchableScript() const
{
	switch (ScriptType)
	{
		case (EScriptType::Python):
			switch (EnvType)
			{
				case (EPythonEnvironmentType::Conda):
					return FLaunchableScript(FString("conda"), FString("run --live-stream -n ") + this->CondaEnvName + FString(" python ") + WithQuotes(this->GetScriptPath().FilePath));

				case (EPythonEnvironmentType::VEnv):
					return FLaunchableScript(this->CustomPythonPath.FilePath, WithQuotes(this->GetScriptPath().FilePath));

				default:
					return FLaunchableScript(FString("python"), WithQuotes(this->GetScriptPath().FilePath));
			}

		default:
			return FLaunchableScript(this->GetScriptPath().FilePath);
	}
}

FScriptSettings::~FScriptSettings()
{
}

FLaunchableScript::FLaunchableScript()
{
	this->ScriptURL = FString("");
	this->Args = FString("");
}

FLaunchableScript::FLaunchableScript(FString ScriptURL)
{
	this->ScriptURL = ScriptURL;
	this->Args = FString("");
}

FLaunchableScript::FLaunchableScript(FString ScriptURL, FString Args)
{
	this->ScriptURL = ScriptURL;
	this->Args = Args;
}

void FLaunchableScript::AppendArgs(FString& AdditionalArgs)
{
	this->Args += (FString(" ") + AdditionalArgs);
}

void FCustomTrainingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	FString Output;

	
	for (auto& Elem : this->Flags)
	{
		ArgBuilder.AddFlag(Elem,true);
	}

	for (auto& Elem : this->Args)
	{
		ArgBuilder.AddStringArg(Elem.Key, Elem.Value);
	}
}

FCustomTrainingSettings::~FCustomTrainingSettings()
{
}

void FRLlibResourceSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	if(this->bUseCustomNumberOfCPUs)
	{
		ArgBuilder.AddIntArg(TEXT("num-cpus"),this->NumCPUs);
	}
	ArgBuilder.AddIntArg(TEXT("num-gpus"),this->NumGPUs);
	ArgBuilder.AddIntArg(TEXT("num-cpus-for-main-process"),this->NumCPUsForMainProcess);

	ArgBuilder.AddIntArg(TEXT("num-learners"),this->NumLearners);
	ArgBuilder.AddIntArg(TEXT("num-gpus-per-learner"),this->NumGPUsPerLearner);
	ArgBuilder.AddIntArg(TEXT("num-cpus-per-learner"),this->NumCPUsPerLearner);
}

FRLlibResourceSettings::~FRLlibResourceSettings()
{
}

void FRLlibCheckpointSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{

	if (this->bSaveFinalModel)
	{
		ArgBuilder.AddFlag(TEXT("save-final-policy"));
		ArgBuilder.AddFlag(TEXT("export-onnx"), this->bExportToONNX);
	}

	if (this->bEnableCheckpoints)
	{
		ArgBuilder.AddFlag(TEXT("enable-checkpoints"));
		ArgBuilder.AddIntArg(TEXT("save-freq"), this->SaveFreq);
	}

	ArgBuilder.AddConditionalStringArg(TEXT("checkpoint-dir"), this->CheckpointDir.Path, !this->CheckpointDir.Path.IsEmpty());
}

FRLlibCheckpointSettings::~FRLlibCheckpointSettings()
{
}

void FSB3CheckpointSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddFlag(TEXT("save-final-policy"), this->bSaveFinalModel);
	

	if (this->bSaveCheckpoints)
	{
		ArgBuilder.AddFlag(TEXT("enable-checkpoints"));
		ArgBuilder.AddIntArg(TEXT("save-freq"), this->SaveFreq);
		ArgBuilder.AddFlag(TEXT("save-replay-buffer"),this->bSaveReplayBuffer);
		ArgBuilder.AddFlag(TEXT("export-onnx"),this->bExportFinalModelToOnnx);
	}

	// Shared options between the two
	if (this->bSaveFinalModel || this->bSaveCheckpoints)
	{
		ArgBuilder.AddStringArg(TEXT("checkpoint-dir"), this->CheckpointDir.Path);
		ArgBuilder.AddFlag(TEXT("save-vecnormalize"), this->bSaveVecNormalize);
		ArgBuilder.AddStringArg(TEXT("name-prefix"), this->NamePrefix);
	}
}

FSB3CheckpointSettings::~FSB3CheckpointSettings()
{
}

void FRLlibLoggingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddIntArg(TEXT("schola-verbosity"), EnvLoggingVerbosity);
	ArgBuilder.AddIntArg(TEXT("rllib-verbosity"), TrainerLoggingVerbosity);
}

FRLlibLoggingSettings::~FRLlibLoggingSettings()
{
}

void FSB3LoggingSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddIntArg(TEXT("schola-verbosity"), EnvLoggingVerbosity);
	ArgBuilder.AddIntArg(TEXT("sb3-verbosity"), TrainerLoggingVerbosity);
	
	if (this->bSaveTBLogs)
	{
		ArgBuilder.AddFlag(TEXT("enable-tensorboard"));
		ArgBuilder.AddStringArg(TEXT("log-dir"),this->LogDir.Path);
		ArgBuilder.AddIntArg(TEXT("log-freq"), this->LogFreq);
		ArgBuilder.AddIntArg(TEXT("callback-verbosity"), this->CallbackVerbosity);
	}
}

FSB3LoggingSettings::~FSB3LoggingSettings()
{
}

void FRLlibResumeSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddConditionalStringArg(TEXT("resume-from"), this->ModelPath.FilePath, bLoadModel);
}

FRLlibResumeSettings::~FRLlibResumeSettings()
{
}

void FSB3ResumeSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddConditionalStringArg(TEXT("load-replay-buffer"), this->ReplayBufferPath.FilePath, bLoadReplayBuffer);
	ArgBuilder.AddConditionalStringArg(TEXT("load-vecnormalize"), this->VecNormalizePath.FilePath, bLoadVecNormalize);
	ArgBuilder.AddConditionalStringArg(TEXT("resume-from"), this->ModelPath.FilePath, bLoadModel);
}

FSB3ResumeSettings::~FSB3ResumeSettings()
{
}

void FRLlibNetworkArchSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	FString ActivationString;
	switch (ActivationFunction)
	{
		case (EActivationFunctionEnum::TanH):
			ActivationString = TEXT("tanh");
			break;

		case (EActivationFunctionEnum::ReLU):
			ActivationString = TEXT("relu");
			break;

		default:
			ActivationString = TEXT("sigmoid");
			break;
	}
	ArgBuilder.AddStringArg(TEXT("activation"),ActivationString);
	ArgBuilder.AddIntArrayArg(TEXT("fcnet-hiddens"), this->FCNetHiddens);
}

FRLlibNetworkArchSettings::~FRLlibNetworkArchSettings()
{
}

void FSB3NetworkArchSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{

	FString ActivationString;
	switch (ActivationFunction)
	{
		case (EActivationFunctionEnum::TanH):
			ActivationString = TEXT("tanh");
			break;

		case (EActivationFunctionEnum::ReLU):
			ActivationString = TEXT("relu");
			break;

		default:
			ActivationString = TEXT("sigmoid");
			break;
	}
	ArgBuilder.AddStringArg(TEXT("activation"), ActivationString);
	ArgBuilder.AddIntArrayArg(TEXT("critic-parameters"), this->CriticParameters);
	ArgBuilder.AddIntArrayArg(TEXT("policy-parameters"), this->PolicyParameters);
}


FSB3NetworkArchSettings::~FSB3NetworkArchSettings()
{

}

void FSB3PPOSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddFloatArg(TEXT("learning-rate"),this->LearningRate);
	ArgBuilder.AddIntArg(TEXT("n-steps"),this->NSteps);
	ArgBuilder.AddIntArg(TEXT("batch-size"),this->BatchSize);
	ArgBuilder.AddIntArg(TEXT("n-epochs"), this->NEpochs);
	
    ArgBuilder.AddFloatArg(TEXT("gamma"), this->Gamma);
    ArgBuilder.AddFloatArg(TEXT("gae-lambda"), this->GAELambda);
	ArgBuilder.AddFloatArg(TEXT("clip-range"), this->ClipRange);

	ArgBuilder.AddFlag("normalize-advantage",this->NormalizeAdvantage);
    ArgBuilder.AddFloatArg(TEXT("ent-coef"), this->EntCoef);
    ArgBuilder.AddFloatArg(TEXT("vf-coef"), this->VFCoef);
    ArgBuilder.AddFloatArg(TEXT("max-grad-norm"), this->MaxGradNorm);

	ArgBuilder.AddFlag(TEXT("use-sde"),this->UseSDE);
	ArgBuilder.AddIntArg(TEXT("sde-sample-freq"),this->SDESampleFreq);
	
}

FSB3PPOSettings::~FSB3PPOSettings()
{
}

void FSB3SACSettings::GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const
{
	ArgBuilder.AddFloatArg(TEXT("learning-rate"),this->LearningRate);
	ArgBuilder.AddIntArg(TEXT("buffer-size"), this->BufferSize);
	ArgBuilder.AddIntArg(TEXT("learning-starts"), this->LearningStarts);
	ArgBuilder.AddIntArg(TEXT("batch-size"), this->BatchSize);
	ArgBuilder.AddFlag(TEXT("optimize-memory-usage"), this->OptimizeMemoryUsage);

	ArgBuilder.AddFloatArg(TEXT("tau"), this->Tau);
	ArgBuilder.AddFloatArg(TEXT("gamma"), this->Gamma);

	ArgBuilder.AddIntArg(TEXT("train-freq"), this->TrainFreq);
	ArgBuilder.AddIntArg(TEXT("gradient-steps"), this->GradientSteps);
	
	
	FString EntCoefString = (this->LearnEntCoef ? FString("auto_") : FString(""))+ FString::SanitizeFloat(this->InitialEntCoef);
	ArgBuilder.AddStringArg(TEXT("ent-coef"), EntCoefString);

	ArgBuilder.AddIntArg(TEXT("target-update-interval"), this->TargetUpdateInterval);

	ArgBuilder.AddStringArg(TEXT("target-entropy"), this->TargetEntropy);
	
	ArgBuilder.AddFlag(TEXT("use-sde"),this->UseSDE);
	ArgBuilder.AddIntArg(TEXT("sde-sample-freq"),this->SDESampleFreq);
}
FSB3SACSettings::~FSB3SACSettings(){};


FScriptArgBuilder& FScriptArgBuilder::AddStringArg(FString ArgName, FString ArgValue)
{
	this->Args.Add(TEXT(" --") + ArgName + TEXT(" ") + ArgValue);
	return *this;
};

FScriptArgBuilder& FScriptArgBuilder::AddFloatArg(FString ArgName, float ArgValue)
{
	return AddStringArg(ArgName, TEXT("\"") + FString::SanitizeFloat(ArgValue) + TEXT("\""));
};

FScriptArgBuilder& FScriptArgBuilder::AddIntArg(FString ArgName, int ArgValue)
{
	return AddStringArg(ArgName, TEXT("\"") + FString::FromInt(ArgValue) + TEXT("\""));
};

FScriptArgBuilder& FScriptArgBuilder::AddFlag(FString FlagName, bool bCondition)
{
	if (bCondition)
	{
		this->Args.Add(TEXT(" --") + FlagName);
	}
	return *this;
};

FScriptArgBuilder& FScriptArgBuilder::AddConditionalStringArg(FString ArgName, FString ArgValue, bool bCondition)
{
	if (bCondition)
	{
		AddStringArg(ArgName, ArgValue);
	}
	return *this;
}
FScriptArgBuilder& FScriptArgBuilder::AddIntArrayArg(FString ArgName, const TArray<int>& ArgValue)
{
	FString ArgString;
	for (const int& Value : ArgValue)
	{
		ArgString += " ";
		ArgString += TEXT("\"") + FString::FromInt(Value) + TEXT("\"");
	}
	return AddStringArg(ArgName, ArgString);
}

FScriptArgBuilder& FScriptArgBuilder::AddPositionalArgument(FString Arg){
	this->Args.Add(TEXT(" ") + Arg);
	return *this;
};

FString FScriptArgBuilder::Build()
{
	FString Output;
	
	for (auto& Arg : this->Args)
	{
		Output += Arg;
	}
	return Output;
};
