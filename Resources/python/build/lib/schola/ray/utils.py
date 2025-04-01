# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Utility functions for working with ray and rllib.
"""
from pathlib import Path
from typing import Dict
import onnx
from ray.rllib.models.torch.torch_modelv2 import TorchModelV2
from ray.rllib.utils.annotations import OldAPIStack, override
import torch.nn as nn
import torch
from gymnasium.spaces import Box, flatdim, Space
from functools import singledispatch
from ray.rllib.policy import Policy

def convert_onnx_for_unreal(model_path : str):
    """
    Convert an ONNX model for use in Unreal by removing unused nodes.

    Parameters
    ----------
    model_path : str
        The path to the ONNX model to convert.
    """
    model = onnx.load(model_path)

    # Remove unused nodes for inference in unreal, namely Identity, state_ins, and state_outs
    for i in range(len(model.graph.node) - 1, -1, -1):
        node = model.graph.node[i]
        if node.op_type == "Identity":
            del model.graph.node[i]

    for i in range(len(model.graph.input) - 1, -1, -1):
        input = model.graph.input[i]
        if input.name == "state_ins":
            del model.graph.input[i]

    for i in range(len(model.graph.output) - 1, -1, -1):
        output = model.graph.output[i]
        if output.name == "state_outs":
            del model.graph.output[i]

    onnx.save(model, model_path)
    onnx.checker.check_model(model)
    print("Converted and save to ", model_path)

@singledispatch
def export_onnx_from_policy(arg, path: str, policy_name=None):
    raise TypeError(f"Cannot export ONNX from Policy/Checkpoint stored as {type(arg)}. Pass a Policy, Dictionary of Policies, or a path to a Policy Checkpoint")

@export_onnx_from_policy.register
def _(arg: Policy, path: str, policy_name=None):
    model_path = path + "/" + policy_name if policy_name else path + "/" + "Policy"
    arg.model = WrappedRLLibModel(arg.model)
    arg.export_model(
        model_path, onnx=9
    )
    convert_onnx_for_unreal(model_path + "/model.onnx")

@export_onnx_from_policy.register
def _(arg: dict, path: str, policy_name=None):
    #policy name is ignored, as the dictionary has them already
    for _policy_name, policy in arg.items():
        export_onnx_from_policy(policy,path,_policy_name)

@export_onnx_from_policy.register 
def _(arg: str, path: str, policy_name=None):
    policy = Policy.from_checkpoint(arg)
    export_onnx_from_policy(policy,path, policy_name)

class WrappedRLLibModel(TorchModelV2, nn.Module):

    def __init__(self, model):
        TorchModelV2.__init__(self, model.obs_space, model.action_space, model.num_outputs, model_config = model.model_config, name=f"Wrapped{model.name}")
        nn.Module.__init__(self)
        self._model = model
        self._mask, self._new_output_dim = self.get_mask(model.action_space)

    @override(TorchModelV2)
    def forward(self,*args,**kwargs):
        model_out = self._model.forward(*args,**kwargs)
        return (model_out[0][:,self._mask], model_out[1])


    @override(TorchModelV2)
    def value_function(self):
        return self._model.value_function()

    def get_mask(self, action_space:Space) -> torch.Tensor:
        mask = torch.as_tensor([True for _ in range(self.num_outputs)])
        curr_dim=0
        new_output_dim = self.num_outputs
        for space_name, space in action_space.items():
            space_size = flatdim(space)
            #remove the 
            if(isinstance(space,Box)):
                mask[curr_dim+space_size:curr_dim+2*space_size] = False
                curr_dim += 2*space_size
                new_output_dim -= space_size
            else:
                curr_dim += space_size

        #convert to 1xN
        return mask, new_output_dim