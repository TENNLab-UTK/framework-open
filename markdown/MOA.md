# The MOA Random Number Generator

This is implemented as an include-only header.  To use this, put the following in your code:

```
#include "utils/MOA.hpp"
```

When you compile, make sure you have `-Iframework/include` in your compilation line, where
`framework` is the directory of the framework.

The header defines a class `neuro::MOA`, which implements the "Mother of All"
random number generator.  I grabbed the code for this in 2016 from
[http://www.agner.org/random](http://www.agner.org/random).    

The goal with the RNG is that it does a good job generating uniformly distributed random
numbers, it is open-source, and it is reliable, meaning it works the same across all
architectures.

# Basic Use

To use this, define an instance of `neuro::MOA`, call `Seed()` and then any number of the
following to generate random numbers:

```
double   Random_Double();                /* Returns a double in the interval [0, 1) */
double   Random_DoubleI();               /* Returns a double in the interval [0, 1] */
double   Random_Normal(double mean, double stddev);   /* What it says */
int      Random_Integer();               /* Returns an integer between 0 and 2^31-1 */
uint32_t Random_32();                    /* Returns a random 32-bit number */
uint64_t Random_64();                    /* Returns a random 64-bit number */
void     Random_128(uint64_t *x);        /* Returns a random 128-bit number */
uint32_t Random_W(int w, int zero_ok);   /* Returns a random w-bit number. (w <= 32)*/
void     Fill_Random_Region (void *reg, int size);   /* reg should be aligned to 4 bytes, but
                                                                   size can be anything. */
```

Just a comment -- in `Random_Double()`, there are 2^(-32)-1 potential values.  In
`Random_DoubleI()`, there is one more value, which is one.  

`Random_Normal()` uses the "Marsaglia polar method" to generate the values.  Please
see [https://en.wikipedia.org/wiki/Marsaglia_polar_method](https://en.wikipedia.org/wiki/Marsaglia_polar_method) and [https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform](https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform) for more information.

--------------------
# Seeding, Checkpoints, Etc.

You seed the RNG with `Seed(uint32_t value, const string &string)`.  
Make the string unique to
your application.  The RNG hashes the string and XOR's it with the value to generate
the seed.

If the `value` is 0, then the `value` is set using `Seed_From_Time()`.   So, if you want
different seeds each run, set the seed to zero.

If you'd rather just specify a value with which to XOR, you can call `Seed_XOR()`.  I
recommend `Seed()` instead of `Seed_XOR()`, but it's up to you.  Don't XOR with zero, please.

`Seed_From_Time()` gets the current time in microseconds, and creates a seed from it.
The exact mechanics are to call `gettimeofday()`, which gives you the current time in 
seconds and microseconds.  We set the seed from seconds, and then massage the bits of
microseconds and XOR them into the seed.

Here are other methods that relate to seeding, etc:

- `uint32_t Hash(const std::string &s)` - Hash the string to a 32-bit unsigned value.
  This uses the "DJB" hash function, which is another effective hash function that I have
  grabbed online.  You can see [My CS202 lecture notes on hashing](https://web.eecs.utk.edu/~jplank/plank/classes/cs202/Notes/Hashing/index.html) for more information.
- `void     Get_State(void *buffer)` - This RNG maintains its state in 5 32-bit numbers.  This
  lets you save those numbers.
- `uint64_t Get_Counter()` - The RNG also maintains a counter from the last time that you called
            `Get_State()`.  This allows you to restore state from the 5 32-bit numbers, and then
            roll forward to where you currently are with the RNG.  The idea is that you call
            `Get_State()` periodicall, and `Get_Counter()` more frequently.
- `void     Set_State(void *buffer, uint64_t counter)` - This will restore the state of the RNG.


--------------------
# Thread Safety

The MOA rng is not thread safe.  If your program uses multiple threads, then use a
different RNG for each thread.
This is how the `app_agent` works.  
