# Format of the "associated data" part of network JSON for use by apps

James S. Plank

When we train an application, we store all parameters that are needed to run the
network in the network itself. To do that, we use the `get_data()` and `set_data()`
methods of the `Network` class.  These store the data in the "Associated_Data" key
of the network's JSON.

These are the following parameters that we store, and therefore that
we read from a network when we want to test it:

Key          | Value
---------    | -------
`app_params` | These are the application parameters used for training.  In `microapp`, this contains the `app_params` JSON updated with the `extra_app_params`.  Typically, when you leave the `app_params` flag blank, the parameters are read from `params` directory in the file whose name is the application, with the extension `.json`.
`proc_params` | These are the processor parameters used for training.  In `microproc`, this contains the `proc_params` JSON updated with the `extra_proc_params`.  Typically, when you leave the `proc_params` flag blank, the parameters are read from `params` directory in the file whose name is the processor, with the extension `.json`.
`encoder_array` | The JSON of the encoder array.
`decoder_array` | The JSON of the decoder array.
`other` | This is a JSON object with three key/value pairs: `sim_time`, which contains the simulation time, `app_name`, which contains the name of the application, and `proc_name`, which contains the name of the processor.  

The applications are (or should be) 
passed the `app_name` when you call their `make()` and `copy()` methods.
For that reason, the `app_name` in the network should match the `app_name` expected by the
application.
