# Useful things to know when using the framework

Author: James S. Plank

The TENNLab software framework is large, and full of options/parameters.  You'll be tempted
to simply use defaults until you get your bearings, but there are quite a few situations
when using defaults is going to get you really bad behavior / performance.  The point of this
markdown page is to help you understand some of the important parameter settings so that
the framework may be more productive for you.

-----------------------------
# Header-only libraries

The framework uses the following header-only libraries for various utilities.  You find
them in the `include/utils` directory off the main framework directory.

- `MOA.hpp` - Random number generation.  Please see [framework_utils_MOA.md](framework_utils_MOA.md).
- `json_helpers.hpp` - We use `nlohmann/json.hpp` for JSON.  This include file has some helper
    routines that we have written.  The most important of these is a routine for checking to make
    sure that your JSON has the right keys, and that they have the right types.  I'll document
    someday.  Until then, just read the header.
- `sys_helpers.hpp` - ChaoHui wrote these to help with various systems tasks.  Again, please give
    it a read if you're interested.

