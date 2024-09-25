# framework-open

Open-source part of the TENNLab Exploratory Neuromorphic Framework

Points of Contact: James S. Plank, Catherine D. Schuman, Charles P. Rizzo.

# Mon Sep 23 11:50:04 EDT 2024

JSP: This is in progress, although you can compile `bin/network_tool` and `bin/processor_tool_risp`,
and run `scripts/test_risp.sh` to make sure that everything is working.
I'm shoring up the documentation, and it will be ready for prime time.

-----------------
# What This Repo Contains

This repo contains the following:

- Simulator for the RISP neuroprocessor.
- `network_tool`: A command-line tool for creating and manipulating spiking neural networks,
  including those for the RISP neuroprocessor.
- `processor_tool_risp`: Command-line tool for simulating the RISP neuroprocessor running networks.
- Spiking neural network format definition.
- C++ `Network` class with supporting methods for creating and manipulating networks.
- C++ `Processor` interface, for applications (like the `processor_tool`) that employ spiking
  neural networks and neuroprocessors.  The RISP simulator implements this interface, but the
  inteface allows for multiple neuroprocessor implementations.

-----------------
# The RISP Neuroprocessor

This is an extremely simple spiking neuroprocessor.  It features leaky integrate-and-fire
neurons and synapses with integer delays.  Its simplicity facilitates efficient implementations,
and there are implementations for RISP in CPU simulation, microcontrollers, and FPGA's.
Here are some links about RISP:

- [The Case for RISP: A Reduced Instruction Spiking Processor](https://neuromorphic.eecs.utk.edu/publications/2022-06-29-the-case-for-risp-a-reduced-instruction-spiking-processor/), a paper describing RISP.
- [The RISP Markdown In This Repo](markdown/risp.md).
- [Open-Source FPGA Implementation of RISP](https://github.com/TENNLab-UTK/fpga).

-----------------
# Documentation

Documentation of everything in this repo is in the [markdown](markdown) directory.
You can typically find what you want there.

-----------------
# "I don't want to touch C++"

You don't have to.  

Although all of the code in this repo is written in C++, your two main tools,
the `network_tool` and `processor_tool`, are command-line tools.  You may interact with them as
command-line programs, and they fit really well with shell scripts and other programming
environments.  So please go through "Getting Started" below -- you don't have to touch any C++.

Now, there is C++ support for network manipulation and for the RISP neuroprocessor.  I suspect
the users for this support are rare, but it is there, and we have it documented.

-----------------
# Getting Started

To get started with this repo, please do one or both of:

- Go through the [Getting Started](markdown/getting_started.md) markdown file.  This has
  a step-by-step walkthrough of using the various tools in this repo.
- Watch the [Getting Started](XX) video.  This is a video where I go through the 
  [Getting Started](markdown/getting_started.md) markdown file.  This allows you to go through
  the steps of learning this software with me explaining as we go.

By the end of this exercise, you should be equipped with the know-how and tools to start creating
and running your own RISP networks!
