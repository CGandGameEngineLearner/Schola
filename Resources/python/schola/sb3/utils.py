# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Utility functions for working with stable baselines 3
"""

from collections import OrderedDict
from typing import Dict, List, Tuple, Union
import torch as th
from stable_baselines3 import PPO
from stable_baselines3.common.base_class import BaseAlgorithm
import os
from argparse import ArgumentParser
import gymnasium as gym
import numpy as np
from stable_baselines3.common.vec_env.base_vec_env import (
    VecEnvObs,
    VecEnv,
    VecEnvWrapper
)
# The below code is adapted from https://github.com/DLR-RM/stable-baselines3/blob/v2.2.1/docs/guide/export.rst

#The MIT License
#
#Copyright (c) 2019 Antonin Raffin
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.

# Modifications Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.

# we don't always include the value network here since we don't need it for inference
class OnnxablePolicy(th.nn.Module):
    """
    A PyTorch Module that wraps a stable baselines policy and extracts the necessary components to export to ONNX.

    Parameters
    ----------
    extractor : th.nn.Module
        The feature extractor from the policy.
    action_net : th.nn.Module
        The action network from the policy.
    value_net : th.nn.Module
        The value network from the policy.
    include_value_net : bool
        Whether to include the value network in the output.
    
    Attributes
    ----------
    extractor : th.nn.Module
        The feature extractor from the policy.
    action_net : th.nn.Module
        The action network from the policy.
    value_net : th.nn.Module
        The value network from the policy.
    include_value_net : bool
        Whether to include the value network in the output
    """
    def __init__(self, extractor: th.nn.Module, action_net : th.nn.Module, value_net: th.nn.Module, include_value_net:bool=False):
        super().__init__()
        self.extractor = extractor
        self.action_net = action_net
        self.value_net = value_net
        self.include_value_net = include_value_net

    def forward(self, x : th.Tensor) -> Union[th.Tensor, Tuple[th.Tensor, th.Tensor]]:
        action_hidden, value_hidden = self.extractor(x)
        if self.include_value_net:
            return self.action_net(action_hidden), self.value_net(value_hidden)
        else:
            return self.action_net(action_hidden)


def save_model_as_onnx(model : BaseAlgorithm, export_path:str) -> None:
    """
    Save a stable baselines model as an ONNX file.

    Parameters
    ----------
    model : stable_baselines3.common.base_class.BaseAlgorithm
        The model to save.
    export_path : str
        The path to save the model to.
    """
   
    new_model = OnnxablePolicy(
        model.policy.mlp_extractor, model.policy.action_net, model.policy.value_net
    )
    #make directories if they don't exist
    directory_path = export_path.rsplit("/",1)[0]
    if not os.path.exists(directory_path):
        os.makedirs(directory_path)

    # Get the input dim from the model
    input_dim = gym.spaces.utils.flatten_space(model.observation_space).shape

    # Export the model to ONNX
    print("Exporting model to ONNX")
    with open(export_path, "w+b") as f:
        th.onnx.export(
            new_model,
            (th.rand(input_dim),),
            f,
            opset_version=9,
            input_names=["input"],
        )
    print("Model exported to ONNX")

# end of adapted code

def convert_ckpt_to_onnx_for_unreal(trainer=PPO, model_path="./ckpt/ppo_final.zip", export_path="./ckpt/OnnxFiles/Model.onnx") -> None:
    """
    Convert a stable baselines model to ONNX for use in Unreal.

    Parameters
    ----------
    trainer : stable_baselines3.common.base_class.BaseAlgorithm
        The trainer to load the model from.
    model_path : str
        The path to the model to convert.
    export_path : str
        The path to save the converted model to.
    
    """
    model = trainer.load(model_path)
    save_model_as_onnx(model,export_path)
    
class VecMergeDictActionWrapper(VecEnvWrapper):
    """
    A vectorized wrapper for merging a dictionary of actions into 1 single action.
    All actions in the dictionary must be of compatible types.

    Parameters
    ----------
    venv : VecEnv
        The vectorized environment being wrapped.
    """

    def __init__(self, venv: VecEnv):
        all_action_spaces = list(venv.action_space.spaces.values())
        assert len(all_action_spaces) > 0, "No Action Spaces to merge."
        action_space = all_action_spaces[0].merge(*all_action_spaces)
        super().__init__(venv=venv, action_space=action_space)

    def reset(self) -> VecEnvObs:
        return self.venv.reset()

    def step(self, action: np.ndarray) -> Tuple[VecEnvObs, np.ndarray, np.ndarray, List[Dict]]:
        return self.venv.step(action)

    def step_async(self, actions: np.ndarray) -> None:
        self.venv.step_async(actions) 

    def step_wait(self) -> Tuple[VecEnvObs, np.ndarray, np.ndarray, List[Dict]]:
        return self.venv.step_wait()