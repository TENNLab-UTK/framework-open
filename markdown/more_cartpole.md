# More Examples of the Cart-Pole Application

These come from the paper, 
"The Cart-Pole Application as a Benchmark for Neuromorphic Computing", by Plank,
Rizzo, White and Schuman.  I'll put a link to the paper when I have it, and when
I refer to it in this markdown file, I'll refer to it as [PRWS2024].

To walk through these examples, or to have instructions on how to use them on
Gymnasium, please see [the main cart-pole example](cartpole_example.md).
Examples 1, 2, 7 and 8 work with Gymnasium pretty much out of the box.  The other
four examples have an addition action, "Do-Nothing," so they require some glue to
work with Gymnasium.

Regardless, for each example, we provide `processor_tool` commands so that
you can see it in action without Gynmasium or any cart-pole application for that
matter.  We also include videos so that you can see it working, along with the
`processor_tool` commands.

---------
## Example 1 - The Main Cart-Pole example in this repo

This is Figure 7(a) from [PRWS2024].  It is a RISP-1+ network, so all synapse
weights are one, and all neuron thresholds are one.  The delays are shown on
the figure:

![PRWS2024-7a.png](images/PRWS2024-7a.png)


