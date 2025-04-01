# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Script to train a Stable Baselines3 model using Schola.
"""
input("Press enter to continue")

import os
import pathlib
from pkgutil import get_data
from typing import Any, Dict, List, Optional, Tuple, TypeVar, Union, Callable, Type

from schola.sb3.utils import convert_ckpt_to_onnx_for_unreal, save_model_as_onnx

from stable_baselines3 import PPO, SAC
from stable_baselines3.common.evaluation import evaluate_policy
from stable_baselines3.common.vec_env import VecNormalize

from schola.scripts.sb3.utils import RewardCallback, CustomProgressBarCallback
from schola.sb3.env import VecEnv
from schola.sb3.utils import VecMergeDictActionWrapper
from schola.core.error_manager import ScholaErrorContextManager
from argparse import ArgumentParser
from schola.sb3.action_space_patch import ActionSpacePatch
from stable_baselines3.common.callbacks import CheckpointCallback
from schola.core.spaces import DictSpace
import traceback
from schola.scripts.common import (
    ActivationFunctionEnum,
    add_unreal_process_args,
    add_checkpoint_args,
    make_unreal_connection,
    ScriptArgs,
)
import torch
from dataclasses import dataclass, field, fields, asdict


@dataclass
class PPOSettings():
    learning_rate: float = 0.0003
    n_steps: int = 2048
    batch_size: int = 64
    n_epochs: int = 10
    gamma: float = 0.99
    gae_lambda: float = 0.95
    clip_range: float = 0.2
    normalize_advantage: bool = True
    ent_coef: float = 0.0
    vf_coef: float = 0.5
    max_grad_norm: float = 0.5
    use_sde: bool = False
    sde_sample_freq: int = -1
    
    @property
    def constructor(self) -> Type[PPO]:
        return PPO 
    
    @property
    def name(self) -> str: 
        return "PPO"
    
    @property
    def critic_type(self) -> str:
        return "vf"

@dataclass 
class SACSettings():
    learning_rate: float = 0.0003  
    buffer_size: int = 1000000  
    learning_starts: int = 100  
    batch_size: int = 256  
    tau: float = 0.005  
    gamma: float = 0.99  
    train_freq: int = 1  
    gradient_steps: int = 1  
    action_noise: any = None  
    replay_buffer_class: any = None  
    replay_buffer_kwargs: dict = None  
    optimize_memory_usage: bool = False  
    ent_coef: any = 'auto'  
    target_update_interval: int = 1  
    target_entropy: any = 'auto'  
    use_sde: bool = False  
    sde_sample_freq: int = -1
    
    @property
    def constructor(self)  -> Type[SAC]: 
        return SAC 
    
    @property
    def name(self) -> str:
        return "SAC"

    @property
    def critic_type(self) -> str:
        return "qf"

@dataclass
class SB3ScriptArgs(ScriptArgs):
    # General Arguments
    timesteps: int = 3000

    #   Misc Arguments
    pbar: bool = False
    disable_eval: bool = False

    # Logging Arguments
    enable_tensorboard: bool = False
    log_dir: str = "./logs"
    log_freq: int = 10
    callback_verbosity: int = 0
    schola_verbosity: int = 0
    sb3_verbosity: int = 1

    # Checkpoint Arguments
    save_replay_buffer: bool = False
    save_vecnormalize: bool = False

    # Resume Arguments
    resume_from: str = None
    load_vecnormalize: str = None
    load_replay_buffer: str = None
    reset_timestep: bool = False

    # Network Architecture Arguments
    policy_parameters: List[int] = None
    critic_parameters: List[int] = None
    activation: ActivationFunctionEnum = ActivationFunctionEnum.ReLU

    # Training Algorithm Arguments
    algorithm_settings: Union[PPOSettings, SACSettings] = field(default_factory=PPOSettings)
    
    @property
    def name_prefix(self):
        return self.name_prefix_override if self.name_prefix_override is not None else self.algorithm_settings.name.lower()


def make_parser() -> ArgumentParser:
    """
    Create an argument parser for launching training with stable baselines 3.

    Returns
    -------
    ArgumentParser
        The argument parser for the script.
    """
    parser = ArgumentParser(prog="Schola Example with SB3")
    parser.add_argument("-t", "--timesteps", type=int, default=3000)
    
    add_unreal_process_args(parser)

    parser.add_argument("--pbar", action="store_true", help="Enable the progress bar. Requires tqdm and rich packages")
    parser.add_argument("--disable-eval", default=False, action="store_true", help="Disable evaluation of the model after training. Useful for short runs that might otherwise hang with an untrained model.")
    logging_group = parser.add_argument_group("Logging Arguments")
    logging_group.add_argument("--enable-tensorboard", action="store_true", help="Enable Tensorboard Logging")
    logging_group.add_argument("--log-dir", type=str, default="./logs", help="Directory to save tensorboard logs, if enabled")
    logging_group.add_argument("--log-freq", type=int, default=10, help="Frequency with which to log to Tensorboard, if enabled")
    logging_group.add_argument("--callback-verbosity", type=int, default=0, help="Verbosity level for any Sb3 callback functions")
    logging_group.add_argument("-scholav", "--schola-verbosity", type=int, default=0, help="Verbosity level for Schola environment logs.")
    logging_group.add_argument("-sb3v", "--sb3-verbosity", type=int, default=1, help="Verbosity level for Stable Baselines3 logs.")

    checkpoint_group = add_checkpoint_args(parser)
    checkpoint_group.add_argument("--save-replay-buffer", action="store_true", help="Save the replay buffer during training, if saving checkpoints")
    checkpoint_group.add_argument("--save-vecnormalize", action="store_true", help="Save the VecNormalize parameters during training, if saving checkpoints")

    resume_group = parser.add_argument_group("Resume Arguments")
    resume_group.add_argument("--resume-from", type=str, default=None, help="Path to a saved model to resume training from")
    resume_group.add_argument("--load-vecnormalize", type=str, default=None, help="Path to a saved VecNormalize parameters to load, if resuming from a checkpoint")
    resume_group.add_argument("--load-replay-buffer", type=str, default=None, help="Path to a saved Replay Buffer to load, if resuming from a checkpoint")
    resume_group.add_argument("--reset-timestep", action="store_true", help="Reset the timestep counter to 0 when resuming from a checkpoint")

    architecture_group = parser.add_argument_group("Network Architecture Arguments")
    
    architecture_group.add_argument(
        "--policy-parameters", nargs="*", type=int, default=None, help="Network architecture for the policy"
    )
    architecture_group.add_argument(
        "--critic-parameters", nargs="*", type=int, default=None, help="Network architecture for the critic. Either the Q-function or the Value-Function depending on algorithm."
    )
    
    architecture_group.add_argument(
        "--activation",
        type=ActivationFunctionEnum,
        default=ActivationFunctionEnum.ReLU,
        help="Activation function to use for the network",
    )
    subparsers = parser.add_subparsers(required=True, help="Choose the algorithm to use")  

    ppo_parser = subparsers.add_parser("PPO", help="Proximal Policy Optimization")
    ppo_parser.add_argument("--learning-rate", type=float, default=0.0003, help="The learning rate for the PPO algorithm")
    ppo_parser.add_argument("--n-steps", type=int, default=2048, help="The number of steps to take in each environment before updating the policy")
    ppo_parser.add_argument("--batch-size", type=int, default=64, help="The number of samples to take from the replay buffer for each update")
    ppo_parser.add_argument("--n-epochs", type=int, default=10, help="The number of epochs to train the policy for each update")
    ppo_parser.add_argument("--gamma", type=float, default=0.99, help="The discount factor for the PPO algorithm")
    ppo_parser.add_argument("--gae-lambda", type=float, default=0.95, help="The GAE lambda value for the PPO algorithm")
    ppo_parser.add_argument("--clip-range", type=float, default=0.2, help="The clip range for the PPO algorithm")
    
    ppo_parser.add_argument("--normalize-advantage", action="store_true", help="Whether to normalize the advantage function")
    ppo_parser.add_argument("--ent-coef", type=float, default=0.0, help="The entropy coefficient for the PPO algorithm")
    ppo_parser.add_argument("--vf-coef", type=float, default=0.5, help="The value function coefficient for the PPO algorithm")
    ppo_parser.add_argument("--max-grad-norm", type=float, default=0.5, help="The maximum gradient norm for the PPO algorithm")
    ppo_parser.add_argument("--use-sde", action="store_true", default=False, help="Whether to use the State Dependent Exploration for the PPO algorithm")
    ppo_parser.add_argument("--sde-sample-freq", type=int, default=-1, help="The frequency at which to sample from the SDE for the PPO algorithm")
    ppo_parser.set_defaults(algorithm_settings_class=PPOSettings)

    sac_parser = subparsers.add_parser("SAC", help="Soft Actor-Critic")
    sac_parser.add_argument("--learning-rate", type=float, default=0.0003, help="The learning rate for the SAC algorithm")
    sac_parser.add_argument("--buffer-size", type=int, default=1000000, help="The size of the replay buffer for the SAC algorithm")
    sac_parser.add_argument("--learning-starts", type=int, default=100, help="The number of steps to take before starting to learn with the SAC algorithm")
    sac_parser.add_argument("--batch-size", type=int, default=256, help="The number of samples to take from the replay buffer for each update")
    sac_parser.add_argument("--tau", type=float, default=0.005, help="The tau value for the SAC algorithm")
    sac_parser.add_argument("--gamma", type=float, default=0.99, help="The discount factor for the SAC algorithm")
    sac_parser.add_argument("--train-freq", type=int, default=1, help="The frequency at which to train the policy for the SAC algorithm")
    sac_parser.add_argument("--gradient-steps", type=int, default=1, help="The number of gradient steps to take for the SAC algorithm")
    sac_parser.add_argument("--optimize-memory-usage", action="store_true", default=False, help="Whether to optimize memory usage for the SAC algorithm")
    sac_parser.add_argument("--ent-coef", default='auto',type=str, help="The entropy coefficient for the SAC algorithm")
    sac_parser.add_argument("--target-update-interval", type=int, default=1, help="The frequency at which to update the target network for the SAC algorithm")
    sac_parser.add_argument("--target-entropy", default='auto',type=str, help="The target entropy for the SAC algorithm")
    sac_parser.add_argument("--use-sde", action="store_true", default=False, help="Whether to use the State Dependent Exploration for the SAC algorithm")
    sac_parser.add_argument("--sde-sample-freq", type=int, default=-1, help="The frequency at which to sample from the SDE for the SAC algorithm")
    sac_parser.set_defaults(algorithm_settings_class=SACSettings)

    return parser


def main(args: SB3ScriptArgs) -> Optional[Tuple[float,float]]:
    """
    Main function for training a Stable Baselines3 model using Schola.

    Parameters
    ----------
    args : SB3ScriptArgs
        The arguments for the script.
    
    Returns
    -------
    Optional[Tuple[float,float]]
        The mean and standard deviation of the rewards if evaluation is enabled, otherwise None.
    """
    if args.pbar:
        try:
            import tqdm
        except:
            print("tqdm not installed. disabling PBar")
            args.pbar = False

    if args.pbar:
        try:
            import rich
        except:
            print("rich not installed. disabling PBar")
            args.pbar = False

    if args.enable_tensorboard:
        try:
            import tensorboard
        except:
            print("tensorboard not installed. Disabling tensorboard logging")
            args.enable_tensorboard = False

    # This context manager redirects GRPC errors into custom error types to help debug
    with ScholaErrorContextManager() as err_ctxt, ActionSpacePatch(
        globals()
    ) as action_patch:

        # make a gym environment
        env = VecEnv(args.make_unreal_connection(), verbosity=args.schola_verbosity)
        
        if args.algorithm_settings.name == "SAC":
            env = VecMergeDictActionWrapper(env)

        model_loaded = False
        if args.resume_from:
            try:
                model = args.algorithm_settings.constructor.load(args.resume_from, env=env)
                model_loaded = True
            except:
                print("Error Loading Model. Training from Scratch")

        if not model_loaded:
            policy_kwargs = None
            if args.activation or args.value_func_parameters or args.policy_parameters:
                policy_kwargs = dict()
                if args.activation:
                    policy_kwargs["activation_fn"] = args.activation.layer

                if args.critic_parameters or args.policy_parameters:
                    # default to nothing
                    policy_kwargs["net_arch"] = dict(vf=[], pi=[], qf=[])

                if args.critic_parameters:
                    policy_kwargs["net_arch"][args.algorithm_settings.critic_type] = args.critic_parameters
                
                if args.policy_parameters:
                    policy_kwargs["net_arch"]["pi"] = args.policy_parameters

            model = args.algorithm_settings.constructor(
                policy = "MultiInputPolicy" if isinstance(env.observation_space, DictSpace) else "MlpPolicy" ,
                env = env,
                verbose = args.sb3_verbosity,
                policy_kwargs = policy_kwargs,
                tensorboard_log = args.log_dir if args.enable_tensorboard else None,
                **asdict(args.algorithm_settings)
            )

        if args.load_vecnormalize:
            if model.get_vec_normalize_env() is None:
                try:
                    VecNormalize.load(args.load_vecnormalize, env)
                except:
                    print("Error Loading saved VecNormalize Parameters. Skipping")
            else:
                print(
                    "Load VecNormalize requested but no VecNormalize Wrapper exists to load to."
                )

        if args.load_replay_buffer:
            if hasattr(model, "replay_buffer"):
                try:
                    model.load_replay_buffer(args.load_replay_buffer)
                except:
                    print("Error Loading saved Replay Buffer. Skipping.")
            else:
                print("Model does not have a Replay Buffer to load to. Skipping.")

        callbacks = []

        if args.enable_tensorboard:
            reward_callback = RewardCallback(
                frequency=args.log_freq, num_envs=env.num_envs
            )
            callbacks.append(reward_callback)

        if args.enable_checkpoints:
            ckpt_callback = CheckpointCallback(
                save_freq=args.save_freq,
                save_path=args.checkpoint_dir,
                name_prefix=args.name_prefix,
                save_replay_buffer=args.save_replay_buffer,
                save_vecnormalize=args.save_vecnormalize,
            )
            callbacks.append(ckpt_callback)

        if args.pbar:
            pbar_callback = CustomProgressBarCallback()
            callbacks.append(pbar_callback)

        # if we loaded a model don't reset the timesteps
        model.learn(
            total_timesteps=args.timesteps,
            callback=callbacks,
            reset_num_timesteps=not model_loaded,
        )

        if args.save_final_policy:
            print("...saving")
            model.save(
                os.path.join(args.checkpoint_dir, f"{args.name_prefix}_final.zip")
            )

            if args.save_vecnormalize and model.get_vec_normalize_env() is not None:
                model.get_vec_normalize_env().save(
                    os.path.join(args.checkpoint_dir, f"{args.name_prefix}_final.zip")
                )

            if args.export_onnx:
                convert_ckpt_to_onnx_for_unreal(model,f"{args.checkpoint_dir}/{args.name_prefix}_final.zip", f"{args.checkpoint_dir}/{args.name_prefix}_final.onnx")

        if not args.disable_eval:
            print("...evaluating the model")
            mean_reward, std_reward = evaluate_policy(
                model, env, n_eval_episodes=10, deterministic=True
            )
            print(f"mean_reward={mean_reward:.2f} +/- {std_reward}")
            env.close()
            return mean_reward, std_reward
        else:
            print("...evaluation disabled. Skipping.")
            env.close()


def get_dataclass_args(args: Dict[str,Any], dataclass : Type[Any] ) -> Dict[str,Any]:
    """
    Get the arguments for a dataclass from a dictionary, potentially containing additional arguments.
    
    Parameters
    ----------
    args : Dict[str,Any]
        The dictionary of arguments.

    dataclass : Type[Any]
        The dataclass to get the arguments for.
    
    Returns
    -------
    Dict[str,Any]
        The arguments for the dataclass.
    """
    return {k: v for k, v in args.items() if k in {f.name for f in fields(dataclass)}}
        
def main_from_cli() -> Optional[Tuple[float,float]]:
    """
    Main function for launching training with stable baselines 3 from the command line.

    Returns
    -------
    Optional[Tuple[float,float]]
        The mean and standard deviation of the rewards if evaluation is enabled, otherwise None.

    See Also
    --------
    main : The main function for launching training with stable baselines 3
    """
    parser = make_parser()
    args = parser.parse_args()
    args_dict = vars(args)
    sb3_args = get_dataclass_args(args_dict,SB3ScriptArgs)
    algorithm_args = get_dataclass_args(args_dict, args_dict["algorithm_settings_class"])
    algorithm_args = args.algorithm_settings_class(**algorithm_args)
    args = SB3ScriptArgs(algorithm_settings=algorithm_args, **sb3_args)
    return main(args)


def debug_main_from_cli() -> None:
    """
    Debug main function for launching training with stable baselines 3 from the command line.

    See Also
    --------
    main_from_cli : The main function for launching training with stable baselines 3 from the command line
    main : The main function for launching training with stable baselines 3
    """
    try:
        main_from_cli()
    except Exception as e:
        traceback.print_exc()
    finally:
        input("Press any key to close:")


if __name__ == "__main__":
    debug_main_from_cli()
