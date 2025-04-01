# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Script to train an rllib model using Schola.
"""
from argparse import ArgumentParser
from schola.ray.utils import export_onnx_from_policy
from typing import List, Optional
import traceback

from schola.ray.env import BaseEnv, sorted_multi_agent_space
from schola.core.env import ScholaEnv

import ray
from ray import air, tune
from ray.rllib.policy.policy import PolicySpec
from ray.rllib.algorithms.ppo import PPOConfig
from ray.rllib.algorithms import ppo
from ray.tune.registry import register_env
import os
from schola.scripts.common import (
    ActivationFunctionEnum,
    add_unreal_process_args,
    add_checkpoint_args,
    ScriptArgs,
)
from dataclasses import dataclass, field
from ray.rllib.policy.policy import Policy

@dataclass
class RLlibArgs(ScriptArgs):
    # Training Arguments
    timesteps: int = 3000
    n_steps: int = 2048
    learning_rate: float = 0.0003
    mini_batch_size: int = 256

    # Logging Arguments
    schola_verbosity: int = 0
    rllib_verbosity: int = 1

    # Resume Arguments
    resume_from: Optional[str] = None

    # Network Architecture Arguments
    fcnet_hiddens: List[int] = field(default_factory=lambda: [512, 512])
    activation: ActivationFunctionEnum = ActivationFunctionEnum.ReLU

    #Resource Arguments

    #defaults to number of vcores, can be set otherwise to limit allocated cores
    num_cpus: Optional[int] = None
    num_learners: Optional[int] = None
    num_cpus_for_main_process: Optional[int] = None
    num_gpus_per_learner: int = 0
    num_cpus_per_learner: int = 1
    num_gpus: Optional[int] = None
    

def make_parser():
    """
    Create an argument parser for launching training with ray.

    Returns
    -------
    ArgumentParser
        The argument parser for the script.
    """
    parser = ArgumentParser(prog="Schola Example with RLlib")
    cwd = os.getcwd()

    parser.add_argument("-t", "--timesteps", type=int, default=3000, help="Number of timesteps to train for")
    parser.add_argument("--n-steps", type=int, default=2048, help="Number of steps to take in each rollout")
    parser.add_argument("--learning-rate", type=float, default=0.0003, help="Learning rate for the PPO algorithm")
    parser.add_argument("--mini-batch-size", type=int, default=256, help="Size of the minibatch for training")

    add_unreal_process_args(parser)

    logging_group = parser.add_argument_group("Logging Arguments")
    logging_group.add_argument("-scholav", "--schola-verbosity", type=int, default=0, help="Verbosity level for the Schola environment")
    logging_group.add_argument("-rllibv", "--rllib-verbosity", type=int, default=1, help="Verbosity level for RLlib")

    checkpoint_group = add_checkpoint_args(parser)

    resume_group = parser.add_argument_group("Resume Arguments")
    resume_group.add_argument("--resume-from", type=str, default=None, help="Path to checkpoint to resume from")

    architecture_group = parser.add_argument_group("Network Architecture Arguments")
    architecture_group.add_argument(
        "--fcnet-hiddens", nargs="+", type=int, default=[512, 512], help="Hidden layer architecture for the fully connected network"
    )
    architecture_group.add_argument(
        "--activation",
        type=ActivationFunctionEnum,
        default=ActivationFunctionEnum.ReLU,
        help="Activation function for the fully connected network",
    )

    resource_group = parser.add_argument_group("Resource Arguments")
    resource_group.add_argument("--num-gpus", type=int, default=None, help="Number of GPUs to use")
    resource_group.add_argument("--num-cpus", type=int, default=None, help="Number of CPUs to use")
    resource_group.add_argument("--num-cpus-per-learner", type=int, default=1, help="Number of CPUs to use per learner process")
    resource_group.add_argument("--num-gpus-per-learner", type=int, default=0, help="Number of GPUs to use per learner process")
    resource_group.add_argument("--num-learners", type=int, default=None, help="Number of learner processes to use")
    resource_group.add_argument("--num-cpus-for-main-process", type=int, default=None, help="Number of CPUs to use for the main process")

    return parser


def main_from_cli() -> tune.ExperimentAnalysis:
    """
    Main function for launching training with ray from the command line.

    Returns
    -------
    tune.ExperimentAnalysis
        The results of the training

    See Also
    --------
    main : The main function for launching training with ray
    """
    parser = make_parser()
    args = parser.parse_args()
    arg_dataclass = RLlibArgs(**vars(args))
    return main(arg_dataclass)


def main(args: RLlibArgs) -> tune.ExperimentAnalysis:
    """
    Main function for launching training with ray.

    Parameters
    ----------
    args : RLlibArgs
        The arguments for the script as a dataclass
    
    Returns
    -------
    tune.ExperimentAnalysis
        The results of the training
    """
    schola_env = ScholaEnv(args.make_unreal_connection(), verbosity=args.schola_verbosity)
    num_agents = len(schola_env.obs_defns[0])
    agent_names = schola_env.agent_display_names[0]
    schola_env.close()
    
    ray.init(num_cpus=args.num_cpus)

    def env_creator(env_config):
        env = BaseEnv(args.make_unreal_connection(), verbosity=args.schola_verbosity)
        return env

    def policy_mapping_fn(agent_id, episode=None, worker=None, **kwargs):
        return agent_names[agent_id]

    register_env("schola_env", env_creator)

    config : PPOConfig = (
        PPOConfig()
        .environment("schola_env", 
                     clip_rewards=False,
                     clip_actions=True)
        .framework("torch")
        .env_runners(
            num_env_runners=0,
            rollout_fragment_length=200,
        )
        .multi_agent(
            policies={
                agent_name: PolicySpec(observation_space=None, action_space=None)
                for agent_name in set(agent_names.values())
            },
            policy_mapping_fn=policy_mapping_fn,
            policies_to_train=None, #default to training all policies
        )
        .resources(
            num_gpus=args.num_gpus,
            num_cpus_for_main_process=args.num_cpus_for_main_process,
        )
        .learners(
            num_learners=args.num_learners,
            num_cpus_per_learner=args.num_cpus_per_learner,
            num_gpus_per_learner=args.num_gpus_per_learner,
        )
        .training(
            lr=args.learning_rate,
            lambda_=0.95,
            gamma=0.99,
            sgd_minibatch_size=args.mini_batch_size,
            train_batch_size=args.n_steps,
            num_sgd_iter=5,
            clip_param=0.2,
            model={
                "fcnet_hiddens": args.fcnet_hiddens,
                "fcnet_activation": args.activation.layer,
                "free_log_std":False, # onnx fails to load if this is set to True
            },
        )
    )

    stop = {
        "timesteps_total": args.timesteps,
    }

    print("Starting training")
    try:
        results = tune.run(
            "PPO",
            config=config,
            stop=stop,
            checkpoint_config=air.CheckpointConfig(
                checkpoint_frequency=args.save_freq if args.enable_checkpoints else 0,
                checkpoint_at_end=args.save_final_policy,
            ),
            restore=args.resume_from,
            verbose=args.rllib_verbosity,
            storage_path=args.checkpoint_dir,
        )
        last_checkpoint = results.get_last_checkpoint()
        
        print("Training complete")
    finally:
        #Always shutdown ray and release the environment from training even if there is an error
        #will reraise the error unless a control flow statement is added
        ray.shutdown()

    if args.export_onnx:
        export_onnx_from_policy(Policy.from_checkpoint(last_checkpoint), results.trials[-1].path)
        print("Models exported to ONNX at ", results.trials[-1].path)
    return results


def debug_main_from_cli() -> None:
    """
    Main function for launching training with ray from the command line, that catches any errors and waits for user input to close.

    See Also
    --------
    main_from_cli : The main function for launching training with ray from the command line
    main : The main function for launching training with ray
    """
    try:
        main_from_cli()
    except Exception as e:
        traceback.print_exc()
    finally:
        input("Press any key to close:")


if __name__ == "__main__":
    debug_main_from_cli()
