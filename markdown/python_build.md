# Python Build

This markdown is a simple walk-through on how to set up, activate, and update the TennLab Python environment. For information about the TennLab framework see the [Getting Started](./getting_started.md) markdown. All functions between the C++ and Python environments are identical.

------------------------------

## Setting Up the Environment

To begin `cd` to the `framework-open` directory. Next, run:

```

UNIX> ./scripts/create_env.sh

```

This will create the environment necessary for implementing the functions within the C++ build. This may take some time.

Once the creation has completed, the Python environment can be acivated through

```

UNIX> . pyframework/bin/activate

```

While this environment is active, any python file that includes the lines:

```

import neuro
import risp

```

Will be able to access the TennLab frameworks functions.

To exit the python environment just do:

```

UNIX> deactivate

```

## Updating the Environment

If changes are made to the pybinds or cmake file the envronment can be updated by once again changing to the `framework-open` directory and running:

```

UNIX> ./scripts/update_env.sh

```
