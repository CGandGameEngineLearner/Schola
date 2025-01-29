# Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
from itertools import chain
from setuptools import setup, find_packages


def get_ray_deps():
    return [
        "ray[tune]==2.35", #Encountering TuneError with ray < 2.30, 2.35 is know to be stable. Other versions might work
        "dm_tree",
        "lz4",
        "scikit-image",
        "pyyaml",
        "scipy",
        "typer",
        "rich"
        ]

def get_sb3_deps():
    return  ["stable-baselines3==2.1.0","tqdm","rich"]
     
def get_docs_deps():
    return [ "sphinx", "breathe", "sphinx_rtd_theme", "sphinx-tabs", "sphinx-copybutton"]

def get_test_deps():
    return ["pytest", "pytest-timeout", "pytest-cov"]

def merge_deps(*dep_lists):
    return list(set(chain.from_iterable(dep_lists)))


def get_all_deps():
    return merge_deps(get_sb3_deps(), get_ray_deps())

if __name__ == "__main__":
    # load readme
    desc = None
    with open("README.MD", "rt") as readme:
        desc = readme.read()
    assert desc != None, "failed to load readme"

    setup(
        name="schola",
        version="1.0",
        author="Advanced Micro Devices, Inc.",
        author_email="alexcann@amd.com",
        packages=find_packages(),
        description=desc,
        install_requires=[
        "protobuf>=3.20",
        "grpcio>=1.51.1",
        "onnx>=1.11, <1.16.2",
        "gymnasium==0.29.1"
        ],
        extras_require={
            "sb3": get_sb3_deps(), 
            "rllib": get_ray_deps(), #these are the ray[rllib] requirements ignoring the gym one
            "all": get_all_deps(),
            "docs": get_docs_deps(),
            "test": get_test_deps(),
        },
        entry_points={
            'console_scripts': [
                'schola-rllib = schola.scripts.ray.launch:debug_main_from_cli',
                'schola-sb3 = schola.scripts.sb3.launch:debug_main_from_cli',
                'schola-build-proto = schola.scripts.utils.compile_proto:main'
            ]
        }
    )
