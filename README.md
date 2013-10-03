# `ioskj`

`ioskj` is a simulation model of the Indian Ocean skipjack tuna fishery for management strategy evaluation.

## Status

`ioskj` is still under active development. It requires several third party C++ libraries and a modern C++ compiler which supports the C++11 standard. At this stage we do not recommend trying to compile it yourself. As the model matures we intend to make it available as precompiled executables for Windows and Linux and/or a package for R.

## Organisation

### C++ files

The main C++ files are:

- `dimensions.hpp` - defines the dimensions used in various model arrays e.g. `Region`, `Age`, `Method`
- `fish.hpp` - contains the `Fish` class representing the fish population dynamic
- `fishing.hpp` - contains the `Fishing` class representing fishing activity
- `data.hpp` - reads in and holds data for use in driving and conditioning the model
- `model.hpp` - contains the `Model` class which puts `Fish`, `Fishing` and the various data classes together
- `main.cpp` - the primary C++ file for compiling a `Model` executable

### `data` folder

The `data` folder includes R and Python scripts for processing source data. See the documentation in those files for more details. The resulting, processed data is outputted to the folder `data\processed-data`.

