# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Script to convert a Stable Baselines3 model to an ONNX model for use in Unreal Engine.
"""
import torch as th
from stable_baselines3 import PPO
import os
from argparse import ArgumentParser
import gymnasium as gym
from schola.sb3.utils import convert_ckpt_to_onnx_for_unreal
from schola.sb3.action_space_patch import ActionSpacePatch


if __name__ == "__main__":

    parser = ArgumentParser(prog="Schola Example with SB3")
    parser.add_argument("--model-path", type=str, default="./ckpt/ppo_final.zip")
    parser.add_argument("--output-path", type=str, default="./ckpt/ppo_final/model.onnx")
    args = parser.parse_args()

    with ActionSpacePatch(globals()) as action_patch:
        # Scripts -> python -> Resources -> Schola -> Plugins -> Project Name -> Content
        convert_ckpt_to_onnx_for_unreal(model_path=args.model_path, export_path=args.output_path)
