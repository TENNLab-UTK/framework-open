-----------------------------
# Header-only libraries

The framework uses the following header-only libraries for various utilities.  You find
them in the `include/utils` directory off the main framework directory.

- `MOA.hpp` - Random number generation.  Please see [MOA.md](MOA.md).
- `json_helpers.hpp` - We use `nlohmann/json.hpp` for JSON.  This include file has some helper
    routines that we have written.  The most important of these is a routine for checking to make
    sure that your JSON has the right keys, and that they have the right types.  I'll document
    someday.  Until then, just read the header.
- `sys_helpers.hpp` - ChaoHui wrote these to help with various systems tasks.  Again, please give
    it a read if you're interested.

