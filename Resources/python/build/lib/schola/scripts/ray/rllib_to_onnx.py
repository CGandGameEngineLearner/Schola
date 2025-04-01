# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Script to convert a Ray model to an ONNX model for use in Unreal Engine.
"""
from ray.rllib.policy.policy import Policy
from schola.ray.utils import convert_onnx_for_unreal, export_onnx_from_policy
from argparse import ArgumentParser
from schola.ray.utils import convert_onnx_for_unreal

if __name__ == "__main__":

    parser = ArgumentParser(prog="Ray-to-Unreal Onnx Parser")
    
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--model-path", type=str, default=None)
    group.add_argument("--policy-checkpoint-path", type=str,default=None)
    group.add_argument("--output-path",type=str, default=None)

    args = parser.parse_args()
    if args.model_path:
        convert_onnx_for_unreal(args.model_path, args.output_path)
    elif args.policy_checkpoint_path:
        policy = Policy.from_checkpoint(args.policy_checkpoint_path)
        export_onnx_from_policy(policy, args.output_path)