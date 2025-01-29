// Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GymConnectors/AbstractGymConnector.h"
#include "Common/LogSchola.h"

#include "Interfaces/IPluginManager.h"
#include "SubsystemSettings.generated.h"

FString WithQuotes(FString Input);

/**
 * @brief A class to build arguments and flags for a CLI script.
 */
struct SCHOLA_API FScriptArgBuilder
{
	
public:
	/** The arguments to be passed to the commnad line script */
	TArray<FString> Args;

	/**
	 * @brief Add a string argument to the command line
	 * @param[in] ArgName The name of the argument
	 * @param[in] ArgValue The value of the argument
	 */
	FScriptArgBuilder& AddStringArg(FString ArgName, FString ArgValue);

	/**
	 * @brief Add a float argument to the command line
	 * @param[in] ArgName The name of the argument
	 * @param[in] ArgValue The value of the argument. Will be converted to a string
	 */
	FScriptArgBuilder& AddFloatArg(FString ArgName, float ArgValue);

	/**
	 * @brief Add an integer argument to the command line
	 * @param[in] ArgName The name of the argument
	 * @param[in] ArgValue The value of the argument. Will be converted to a string
	 */
	FScriptArgBuilder& AddIntArg(FString ArgName, int ArgValue);

	/**
	 * @brief Add a flag to the command line
	 * @param[in] FlagName The name of the flag
	 * @param[in] bCondition Whether the flag should be added. Useful programmatically setting conditional flags.
	 */
	FScriptArgBuilder& AddFlag(FString FlagName, bool bCondition = true);

	/**
	 * @brief Add a string argument to the command line if a condition is met
	 * @param[in] ArgName The name of the argument
	 * @param[in] ArgValue The value of the argument
	 * @param[in] bCondition Whether the argument should be added. Useful programmatically setting conditional arguments.
	 */
	FScriptArgBuilder& AddConditionalStringArg(FString ArgName, FString ArgValue, bool bCondition);
	
	/**
	 * @brief Add an integer array argument to the command line
	 * @param[in] ArgName The name of the argument
	 * @param[in] ArgValue The value of the argument
	 * @note The array will be converted to a string by converting to a string array and putting spaces between each value
	 */
	FScriptArgBuilder& AddIntArrayArg(FString ArgName, const TArray<int>& ArgValue);
	
	/**
	 * @brief Add a positional argument (i.e. does not append - infront of the name)
	 * @param[in] Arg The argument to add
	 */
	FScriptArgBuilder& AddPositionalArgument(FString Arg);

	/**
	 * @brief Construct the command line string from the added arguments
	 * @return The command line string
	 */
	FString Build();
};

/**
 * @brief A struct to hold the configuration of a launchable script.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FLaunchableScript
{
	GENERATED_BODY()

public:
	/**
	 * @brief Construct a launchable script with no arguments or URL. Will fail to launch unless ScriptURL is set
	 */
	FLaunchableScript();

	/**
	 * @brief Construct a launchable script with a file URL
	 * @param[in] ScriptURL The URL of the script to launch
	 */
	FLaunchableScript(FString ScriptURL);

	/**
	 * @brief Construct a launchable script with a file URL and arguments
	 * @param[in] ScriptURL The URL of the script to launch
	 * @param[in] Args The arguments to pass to the script
	 */
	FLaunchableScript(FString ScriptURL, FString Args);

	/** A path to the script to be launched */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Networking URL")
	FString ScriptURL;

	/** The arguments to be passed to the script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Networking URL")
	FString Args;

	/** 
	 * @brief Append additional arguments to the script 
	 * @param[in] AdditionalArgs The arguments to append to the script
	 */
	void AppendArgs(FString& AdditionalArgs);

	/**
	 * @brief Launch the script
	 * @return The process handle of the launched script
	 */
	FProcHandle LaunchScript() const;
};

/**
 * @brief A struct to hold settings relating to external communication (e.g. sockets)
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FCommunicatorSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Communicator Settings")
	FString Address = FString("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Communicator Settings")
	int Port = 8000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=1), Category = "Communicator Settings")
	int Timeout = 30;

};

/**
 * @brief The type of a training script.
 */
UENUM(BlueprintType)
enum class EScriptType : uint8
{
	Python UMETA(DisplayName = "Python"),
	Other  UMETA(DisplayName = "Other"),
};

/**
 * @brief The type of a python environment.
 */
UENUM(BlueprintType)
enum class EPythonEnvironmentType : uint8
{
	/** The default system python */
	Default UMETA(DisplayName = "Default Python Env"),
	/** A conda environment with a specified name */
	Conda	UMETA(DisplayName = "Conda Env"),
	/** A virtual environment with a specified path */
	VEnv	UMETA(DisplayName = "Custom Python Path"),
};

/**
 * @brief The type of a python training script to use.
 */
UENUM(BlueprintType)
enum class EPythonScript : uint8
{
	/** use the SB3 launch script included with Schola */
	SB3	   UMETA(DisplayName = "Builtin SB3 Training Script"),
	/** use the RLlib launch script included with Schola */
	RLLIB  UMETA(DisplayName = "Builtin RLlib Training Script"),
	/** use a custom launch script set by the user*/
	Custom UMETA(DisplayName = "Custom Training Script"),
};

/**
 * @brief Abstract class for any training settings
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FTrainingSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Generate the training arguments for the script using the ArgBuilder
	 * @param[in] Port The port to use for the script
	 * @param[in] ArgBuilder The builder to use to generate the arguments
	 * @note port is supplied as it is a common argument to pass to scripts, and is set at a high level but might be needed by specific subsettings
	 */
	virtual void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FTrainingSettings() {};
};

/**
 * @brief A struct to hold settings for a custom training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FCustomTrainingSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The path to the script to launch */
	UPROPERTY(Config, EditAnywhere, Category = "Script Settings|Custom Script")
	FFilePath LaunchScript;

	/** The arguments to pass to the script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Script Settings|Custom Script")
	TMap<FString, FString> Args;

	/** The flags to pass to the script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Script Settings|Custom Script")
	TArray<FString> Flags;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FCustomTrainingSettings();
};

// RLLib Settings
/**
 * @brief A struct to hold logging settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibLoggingSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The verbosity of the environment logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "2"), Category = "Logging Settings")
	int32 EnvLoggingVerbosity = 0;

	/** The verbosity of the trainer logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "2"), Category = "Logging Settings")
	int32 TrainerLoggingVerbosity = 1;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibLoggingSettings();
};

/**
 * @brief An enumeration of activation functions neural networks
 */
UENUM(BlueprintType)
enum class EActivationFunctionEnum : uint8
{
	ReLU	UMETA(DisplayName = "ReLU"),
	Sigmoid UMETA(DisplayName = "Sigmoid"),
	TanH	UMETA(DisplayName = "TanH")
};

/**
 * @brief A struct to hold network architecture settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibNetworkArchSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The activation function to use in the neural network */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
	EActivationFunctionEnum ActivationFunction = EActivationFunctionEnum::ReLU;

	/** The number and width of hidden layers in the neural network */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1), Category = "Architecture Settings")
	TArray<int> FCNetHiddens = { 512, 512 };

	/** The size of each minibatch */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1), Category = "Architecture Settings")
	int MinibatchSize = 256;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibNetworkArchSettings();
};

/**
 * @brief A struct to hold resource settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibResourceSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	
	/** The number of GPUs to use for training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Resource Settings")
	int NumGPUs = 0;

	/** Whether to use a custom number of CPUs for training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Settings")
	bool bUseCustomNumberOfCPUs = false;

	/** The maximum number of CPUs/workers to use for training. If empty uses number of vCPUs on system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveFinalModel"), Category = "Resource Settings")
	int NumCPUs = 1;

	/** The number of learner processes to use for training. Set to 0 to run in main process */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0), BlueprintReadOnly, Category = "Resource Settings")
	int NumLearners = 0;

	/** The number of GPUs to use for the learner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0), Category = "Resource Settings")
	int NumGPUsPerLearner = 0;

	/** The number of CPUs to use for the learner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1), Category = "Resource Settings")
	int NumCPUsPerLearner = 1;

	/** The number of CPUs to use for the main process */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1), Category = "Resource Settings")
	int NumCPUsForMainProcess = 1;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibResourceSettings();
};

/**
 * @brief A struct to hold checkpoint settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibCheckpointSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** Whether to save the final output of training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Settings")
	bool bSaveFinalModel = true;

	/** Whether to export the final model to ONNX automatically */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveFinalModel"), Category = "Checkpoint Settings")
	bool bExportToONNX = false;

	/** Whether to save checkpoints during training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Settings")
	bool bEnableCheckpoints = false;

	/** How frequently should we make checkpoints */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableCheckpoints"), Category = "Checkpoint Settings")
	int SaveFreq = 1000;

	/** The directory to save checkpoints to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Settings")
	FDirectoryPath CheckpointDir;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibCheckpointSettings();
};

/**
 * @brief A struct to hold resume settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibResumeSettings
{
	GENERATED_BODY()

public:
	/** Whether to load a model from a file */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Resume Settings")
	bool bLoadModel = false;

	/** The path to the model to load, if we are loading a model */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bLoadModel", DisplayName = "Resume From Policy Saved to:"), Category = "Resume Settings")
	FFilePath ModelPath;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibResumeSettings();
};

/**
 * @brief A struct to hold all the settings for an RLlib training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FRLlibTrainingSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The number of timesteps to train for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Timesteps = 8000;

	/** The logging settings for the training script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRLlibLoggingSettings LoggingSettings;

	/** The checkpoint settings for the training script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRLlibCheckpointSettings CheckpointSettings;

	/** The resume settings for the training script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRLlibResumeSettings ResumeSettings;

	/** The network architecture settings for the training script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRLlibNetworkArchSettings NetworkArchitectureSettings;

	/** The resource settings for the training script */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRLlibResourceSettings ResourceSettings;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FRLlibTrainingSettings();
};

//SB3 Settings

/**
 * @brief An enumeration of training algorithms supported by SB3
 */
UENUM()
enum class ETrainingAlgorithm
{
	PPO,
	SAC
};

/**
 * @brief A struct to hold logging settings for an SB3 training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3LoggingSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The verbosity of the environment logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "2"), Category = "Logging Settings")
	int32 EnvLoggingVerbosity = 0;

	/** The verbosity of the trainer logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "2"), Category = "Logging Settings")
	int32 TrainerLoggingVerbosity = 0;

	/* Logging related arguments */

	/** Whether to save training logs with tensorboard */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Script Settings|Python Script|SB3|Logging Settings")
	bool bSaveTBLogs = true;

	/** The frequency to save tensorboard logs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveTBLogs"), Category = "Logging Settings")
	int LogFreq = 10;

	/** The directory to save logs to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveTBLogs"), Category = "Logging Settings")
	FDirectoryPath LogDir;

	/** The verbosity of the callback logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveTBLogs", ClampMin = "0", ClampMax = "2"), Category = "Logging Settings")
	int CallbackVerbosity = 1;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3LoggingSettings();
};

/**
 * @brief A struct to hold network architecture settings for an SB3 training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3NetworkArchSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:

	/** The activation function to use in the neural network */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Settings")
	EActivationFunctionEnum ActivationFunction = EActivationFunctionEnum::ReLU;

	/** The number and width of hidden layers in the Critic. Applied to either the Q-Function or Value-Function */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0), Category = "Architecture Settings")
	TArray<int> CriticParameters = { 256, 256 };

	/** The number and width of hiddent layers in the policy network */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0), Category = "Architecture Settings")
	TArray<int> PolicyParameters = { 256, 256 };

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3NetworkArchSettings();
};
/**
 * @brief A struct to hold checkpoint settings for an SB3 training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3CheckpointSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** Whether to save checkpoints during training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Settings")
	bool bSaveCheckpoints = false;

	/** Whether to save the final model */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint Settings")
	bool bSaveFinalModel = true;

	/** Whether to save the VecNormalize parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveCheckpoints || bSaveFinalModel"), Category = "Checkpoint Settings")
	bool bSaveVecNormalize = false;

	/** Whether to save the replay buffer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveCheckpoints"), Category = "Checkpoint Settings")
	bool bSaveReplayBuffer = false;

	/** Whether we should just create a checkpoint, or if we should also export the final model out to ONNX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveFinalModel"), Category = "Checkpoint Settings")
	bool bExportFinalModelToOnnx = true;

	/** The directory to save checkpoints to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveCheckpoints || bSaveFinalModel"), Category = "Checkpoint Settings")
	FDirectoryPath CheckpointDir;

	/** The frequency to save checkpoints */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveCheckpoints"), Category = "Checkpoint Settings")
	int SaveFreq = 1000;

	/** The prefix to use for the checkpoint files */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bSaveCheckpoints || bSaveFinalModel"), Category = "Checkpoint Settings")
	FString NamePrefix = FString("ppo");

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3CheckpointSettings();
};

/**
 * @brief A struct to hold resume settings for an SB3 training script
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3ResumeSettings
{
	GENERATED_BODY()

public:
	/** Whether to load a model from a file */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Resume Settings")
	bool bLoadModel = false;

	/** The path to the model to load, if we are loading a model */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bLoadModel", FilePathFilter = "zip", DisplayName = "Resume From Policy Saved to:"), Category = "Resume Settings")
	FFilePath ModelPath;

	/** Whether to load a replay buffer from a file */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Resume Settings")
	bool bLoadReplayBuffer = false;

	/** The path to the replay buffer to load, if we are loading a replay buffer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bLoadReplayBuffer", FilePathFilter = "pkl", DisplayName = "Load Replay Buffer Saved to:"), Category = "Resume Settings")
	FFilePath ReplayBufferPath;

	/** Whether to load VecNormalize parameters from a file */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle), Category = "Resume Settings")
	bool bLoadVecNormalize = false;

	/** The path to the VecNormalize parameters to load, if we are loading them */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bLoadVecNormalize", FilePathFilter = "pkl", DisplayName = "Load VecNormalize Parameters Saved to:"), Category = "Resume Settings")
	FFilePath VecNormalizePath;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3ResumeSettings();
};

/**
 * @brief A struct to hold PPO settings for an SB3 training script
 * @note This is a partial implementation of the PPO settings, and is not exhaustive
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3PPOSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The learning rate for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LearningRate = 0.0003;

	/** The number of steps to take between training steps */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int NSteps = 2048;

	/** The batch size to use during gradient descent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BatchSize = 64;

	/** The number of epochs to train for each training step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int NEpochs = 10;

	/** The gamma value for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Gamma = 0.99;

	/** The Generalized Advantage Estimate Lambda value for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GAELambda = 0.95;

	/** The clip range for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ClipRange = 0.2;

	/** Should we normalize the advantage values */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool NormalizeAdvantage = true;

	/** The entropy coefficient for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EntCoef = 0.0;

	/** The value function coefficient for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float VFCoef = 0.05;

	/** The maximum gradient norm for the PPO algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxGradNorm = 0.5;

	/** Should we use state dependent entropy noise */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool UseSDE = false;

	/** The frequency to sample the state dependent entropy noise */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int SDESampleFreq = -1;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3PPOSettings();
};

/**
 * @brief A struct to hold SAC settings for an SB3 training script
 * @note This is a partial implementation of the SAC settings, and is not exhaustive
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3SACSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The learning rate for the SAC algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LearningRate = 0.0003;

	/** The buffer size for the SAC algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BufferSize = 1000000;

	/** The number of steps to take before learning starts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int LearningStarts = 100;

	/** The batch size to use during gradient descent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BatchSize = 256;

	/** The Tau value for the SAC algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Tau = 0.005;

	/** The gamma value for the SAC algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Gamma = 0.99;

	/** The frequency to update the target network, in steps */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TrainFreq = 1;

	/** The number of gradient steps to take during training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GradientSteps = 1;

	/** Optimize memory usage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool OptimizeMemoryUsage = false;

	/** Should we learn the entropy coefficient during training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool LearnEntCoef = true;

	/** The initial entropy coefficient for the SAC algorithm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InitialEntCoef = 1.0;

	/** The interval at which we update the target network */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TargetUpdateInterval = 1;

	/** The target entropy for the SAC algorithm. use auto to learn the target entropy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString TargetEntropy = "auto";

	/** Use state dependent entropy noise */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool UseSDE = false;

	/** The frequency to sample the state dependent entropy noise */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int SDESampleFreq = -1;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3SACSettings();
};
/**
 * @brief A struct to hold all the settings for an SB3 training script
 * @note This is a partial implementation of the SB3 settings, and is not exhaustive
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FSB3TrainingSettings : public FTrainingSettings
{
	GENERATED_BODY()

public:
	/** The number of timesteps to train for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Timesteps = 8000;

	/* Logging related arguments */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSB3LoggingSettings LoggingSettings;

	/** Checkpoint related arguments */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSB3CheckpointSettings CheckpointSettings;

	/** Resume related arguments */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSB3ResumeSettings ResumeSettings;

	/** Network architecture related arguments */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSB3NetworkArchSettings NetworkArchitectureSettings;

	/** Display a progress bar during training */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDisplayProgressBar = true;

	/** The algorithm to use during training (e.g. SAC, PPO) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETrainingAlgorithm Algorithm = ETrainingAlgorithm::PPO;

	/** PPO specific settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Algorithm==ETrainingAlgorithm::PPO", EditConditionHides, DisplayName = "PPO Algorithm Settings"))
	FSB3PPOSettings PPOSettings;

	/** SAC specific settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Algorithm==ETrainingAlgorithm::SAC", EditConditionHides, DisplayName = "SAC Algorithm Settings"))
	FSB3SACSettings SACSettings;

	void GenerateTrainingArgs(int Port, FScriptArgBuilder& ArgBuilder) const;

	virtual ~FSB3TrainingSettings();
};

/**
 * @brief All settings for autolaunching a script when starting the game.
 */
USTRUCT(BlueprintType)
struct SCHOLA_API FScriptSettings
{
	GENERATED_BODY()

public:
	
	/** The type of the script to run (e.g. Python, or Other CLI script) */
	UPROPERTY(Config, EditAnywhere)
	EScriptType ScriptType = EScriptType::Python;

	/** The Python environment to use when running a python script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Python", EditConditionHides), Category = "Script Settings|Python Env Settings")
	EPythonEnvironmentType EnvType = EPythonEnvironmentType::Default;

	/** The name of the conda environment to run the python script in */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "EnvType==EPythonEnvironmentType::Conda && ScriptType==EScriptType::Python", EditConditionHides), Category = "Script Settings|Python Env Settings")
	FString CondaEnvName;

	/** The path to a python executable if using a non-default executable */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "EnvType==EPythonEnvironmentType::VEnv && ScriptType==EScriptType::Python", EditConditionHides), Category = "Script Settings|Python Env Settings")
	FFilePath CustomPythonPath;

	/** The type of python script to run, options are scripts builtin to Schola, or a user provided script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Python", EditConditionHides), Category = "Script Settings|Python Script")
	EPythonScript PythonScriptType = EPythonScript::SB3;

	/** Settings to use when running a custom python script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Python && PythonScriptType==EPythonScript::Custom", EditConditionHides), Category = "Script Settings|Python Script")
	FCustomTrainingSettings CustomPythonScriptSettings;

	/** Settings to use when running a custom script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Other", EditConditionHides), Category = "Script Settings|Custom Script")
	FCustomTrainingSettings CustomScriptSettings;

	/** Settings to use when running the builtin SB3 script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Python && PythonScriptType==EPythonScript::SB3", EditConditionHides, DisplayName = "Builtin SB3 Settings"), Category = "Script Settings|Python Script")
	FSB3TrainingSettings SB3Settings;

	/** Settings to use when running the builtin RLlib script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "ScriptType==EScriptType::Python && PythonScriptType==EPythonScript::RLlib", EditConditionHides, DisplayName = "Builtin RLlib Settings"), Category = "Script Settings|Python Script")
	FRLlibTrainingSettings RLlibSettings;

	FString GetTrainingArgs(int Port) const;

	/**
	 * @brief Get the path to the script to run
	 * @return The path to the script to run
	 */
	FFilePath GetScriptPath() const;

	FLaunchableScript GetLaunchableScript() const;

	virtual ~FScriptSettings();
};

/**
 * @brief A class to make UERL subsystem settings available in the Game tab of project settings
 */
UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Schola Settings"))
class SCHOLA_API UScholaManagerSubsystemSettings : public UObject
{
	GENERATED_BODY()

public:
	UScholaManagerSubsystemSettings();

	/** The class of the gym connector to use */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	TSubclassOf<UAbstractGymConnector> GymConnectorClass;

	/** Whether to run the script on play. Can be overriden by a CLI arg. */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bRunScriptOnPlay = false;

	/** The settings for the script */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "bRunScriptOnPlay", ShowOnlyInnerProperties), Category = "Script Settings")
	FScriptSettings ScriptSettings;

	/** The settings for communication */
	UPROPERTY(Config, EditAnywhere, meta = (ShowOnlyInnerProperties), Category = "Communicator Settings")
	FCommunicatorSettings CommunicatorSettings;

	FLaunchableScript GetScript() const;
};
