# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.

"""
Common utility functions and classes for use in Schola scripts.
"""
from enum import Enum
from typing import Optional
import torch
from schola.core.unreal_connections import StandaloneUnrealConnection, UnrealEditorConnection
import os
from dataclasses import dataclass
import argparse

class ActivationFunctionEnum(str, Enum):
    """
    Activation functions for neural networks.

    Attributes
    ----------
    ReLU : Tuple[str, torch.nn.ReLU]
        Rectified Linear Unit activation function.
    Sigmoid : Tuple[str, torch.nn.Sigmoid]
        Sigmoid activation function.
    TanH : Tuple[str, torch.nn.Tanh]
        Hyperbolic Tangent activation function.
    """
    def __new__(cls, value, subcls):
        obj = str.__new__(cls, [value])
        obj._value_ = value
        obj.layer = subcls
        return obj

    ReLU = ("relu", torch.nn.ReLU)
    Sigmoid = ("sigmoid", torch.nn.Sigmoid)
    TanH = ("tanh", torch.nn.Tanh)
    

def add_checkpoint_args(parser : argparse.ArgumentParser) -> argparse._ArgumentGroup:
    """
    Add checkpoint arguments to the parser.

    Parameters
    ----------
    parser : argparse.ArgumentParser
        The parser to which the arguments will be added.
    
    Returns
    -------
    argparse._ArgumentGroup
        The checkpoint group that was added to the parser. Can be used to add more arguments
    """
    checkpoint_group = parser.add_argument_group("Checkpoint Arguments")
    checkpoint_group.add_argument("--enable-checkpoints", action="store_true", help="Enable saving checkpoints")
    checkpoint_group.add_argument("--checkpoint-dir", type=str, default=os.getcwd() + "/ckpt", help="Directory to save checkpoints")
    checkpoint_group.add_argument("--save-freq", type=int, default=100000, help="Frequency with which to save checkpoints")
    checkpoint_group.add_argument("--name-prefix", type=str, default=None, dest="name_prefix_override", help="Override the name prefix for the checkpoint files (e.g. SAC, PPO, etc.)")
    checkpoint_group.add_argument("--export-onnx", action="store_true", help="Whether to export the model to ONNX format instead of just saving a checkpoint")
    checkpoint_group.add_argument("--save-final-policy", action="store_true", help="Whether to save the final policy after training is complete")
    return checkpoint_group
    

def add_unreal_process_args(parser : argparse.ArgumentParser) -> argparse._ArgumentGroup:
    """
    Add Unreal Engine process arguments to the parser.

    Parameters
    ----------
    parser : argparse.ArgumentParser
        The parser to which the arguments will be added.
    
    Returns
    -------
    argparse._ArgumentGroup
        The Unreal Engine process group that was added to the parser. Can be used to add more arguments.
    """
    unreal_process_group = parser.add_argument_group("Unreal Process Arguments")
    unreal_process_group.add_argument("--launch-unreal",action="store_true", help="Flag indicating if the script should launch a standalone Unreal Engine process")
    unreal_process_group.add_argument("--unreal-path",default=None, type=str, help="Path to the Unreal Engine executable, when launching a standalone Unreal Engine process")
    unreal_process_group.add_argument("--headless", action="store_true", help="Flag indicating if the standalone Unreal Engine process should run in headless mode")
    unreal_process_group.add_argument("-p", "--port", type=int, default=None, help="Port to connect to the Unreal Engine process, if None an open port will be automatically selected when running standalone. Port is required if connecting to an existing Unreal Engine process.")
    unreal_process_group.add_argument("--map", type=str, default=None, help="Map to load when launching a standalone Unreal Engine process")
    unreal_process_group.add_argument("--fps", type=int, default=None, help="Fixed FPS to use when running standalone, if None no fixed timestep is used")
    unreal_process_group.add_argument("--disable-script",action="store_true", help="Flag indicating if the autolaunch script setting in the Unreal Engine Schola Plugin should be disabled. Useful for testing.")
    return unreal_process_group

@dataclass
class ScriptArgs:
    """
    Arguments for included scripts

    Attributes
    ----------
    enable_checkpoints : bool
        Enable saving checkpoints.
    checkpoint_dir : str
        Directory to save checkpoints.
    save_freq : int
        Frequency with which to save checkpoints.
    name_prefix_override : str
        Override the name prefix for the checkpoint files (e.g. SAC, PPO, etc.)
    export_onnx : bool
        Whether to export the model to ONNX format instead of just saving a checkpoint.
    save_final_policy : bool
        Whether to save the final policy after training is complete.
    launch_unreal : bool
        Flag indicating if the script should launch a standalone Unreal Engine process.
    port : int, optional
        Port to connect to the Unreal Engine process, if None an open port will be automatically selected when running standalone. Port is required if connecting to an existing Unreal Engine process.
    unreal_path : str, optional
        Path to the Unreal Engine executable. Required when launching a standalone Unreal Engine process.
    headless : bool
        Flag indicating if the standalone Unreal Engine process should run in headless mode.
    map : str, optional
        Map to load when launching a standalone Unreal Engine process.
    fps : int, optional
        Fixed FPS to use when running standalone, if None no fixed timestep is used.
    disable_script : bool
        Flag indicating if the autolaunch script setting in the Unreal Engine Schola Plugin should be disabled. Useful for testing.
    """
    # Checkpoint Arguments
    enable_checkpoints: bool = False
    checkpoint_dir: str = os.getcwd() + "/ckpt"
    save_freq: int = 100000
    name_prefix_override: str = None   
    export_onnx: bool = False
    save_final_policy: bool = False

    # Unreal Process Arguments
    launch_unreal: bool = False
    #default is automatically selected port
    port: Optional[int] = None
    # must be set if launch_unreal is true
    unreal_path: Optional[str] = None
    #Note these arguments only apply if running in standalone mode
    headless: bool = False
    map: Optional[str] = None
    fps:Optional[int] = None
    disable_script:bool = False

    def make_unreal_connection(self):
        """
        Create an Unreal Engine connection based on the script arguments.

        Returns
        -------
        UnrealConnection
            The Unreal Engine connection to use for the script.
        """
        if self.launch_unreal:
           return StandaloneUnrealConnection("localhost", self.unreal_path, self.headless, port=self.port, map=self.map, set_fps=self.fps, disable_script=self.disable_script)
        else:
            return UnrealEditorConnection("localhost", self.port)

def make_unreal_connection(args:ScriptArgs):
    """
    Create an Unreal Engine connection based on the script arguments.

    Parameters
    ----------
    args : ScriptArgs
        The script arguments to use for creating the Unreal Engine connection.

    Returns
    -------
    UnrealConnection
        The Unreal Engine connection to use for the script.
    """
    return args.make_unreal_connection()

