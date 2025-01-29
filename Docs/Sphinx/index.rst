Schola Documentation
====================

The Schola project is an effort to build a toolkit/plugin for controlling Objects in Unreal with Reinforcement Learning. 
It provides tools to help the user create Environments, define Agents, 
connect to python based RL Frameworks (e.g. Gym, RLLib or Stable Baselines 3), and power NPCs with RL during games.

.. toctree::
   :maxdepth: 2
   :caption: Getting Started  

   guides/setup_schola
   guides/setup_examples
   guides/running_schola


.. toctree::
   :maxdepth: 2
   :caption: Example Environments

   guides/example_one
   guides/example_two

API Documentation
-----------------

.. toctree::
   :hidden:
   :caption: Python API Documentation

   API/python_core
   API/python_extensions
   API/python_generated
   

Python
~~~~~~

Core
^^^^

.. autosummary::
   :recursive:

   schola.core

Extensions
^^^^^^^^^^

.. autosummary::
   :recursive:

   schola.sb3
   schola.ray
   schola.gym

Scripts
^^^^^^^

.. autosummary::
   :recursive:

   schola.scripts

Generated gRPC Code
^^^^^^^^^^^^^^^^^^^

.. autosummary::
   :recursive:

   schola.generated

Unreal
~~~~~~

.. toctree::
   :maxdepth: 2
   :caption: Unreal API Documentation

   API/Unreal/classlist
   API/Unreal/structlist
   API/Unreal/namespacelist

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
