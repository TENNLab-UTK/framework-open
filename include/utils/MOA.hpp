/* JSP: 10/2024 -- Just using Mersenne Twister from the C++ standard library.  */

#pragma once
#include <stdio.h>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <sys/time.h>
#include <cstdint>
#include <random>

namespace neuro
{

class MOA {
  public:
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
    uint32_t Seed_From_Time();               /* Generates a random-ish 32-bit number from the
                                                current time in milliseconds. */
    /* You can seed the RNG with an unsigned integer Seed().
       The state of the RNG is held in 20 bytes, which you can access with Get_State().
       We also maintain a counter, which is reset when you call Get_State(), and then incremented
       with every call (potentially multiple times).  You can get the counter with Get_Counter().
       Then, you can reset the RNG to a known state with Set_State(), that takes the 20-byte state
       plus a counter, and resets the state to that point. */

    void     Seed(uint32_t seed, const std::string &name);  /* Seed the RNG.  The name will be 
                                                               hashed and XOR'd with the seed.  If
                                                               the seed is 0, then we use
                                                               Seed_From_Time(). */
    void     Seed_XOR(uint32_t seed, uint32_t hash);  /* Seed and XOR with this value (for
                                                        applications that used to do this). */
    uint32_t Hash(const std::string &s);     /* Return the DJB hash of the string. */
    void     Get_State(void *buffer);        /* Copies internal state & resets an internal counter. 
                                                       The buffer should be >= 20 bytes. */
    uint64_t Get_Counter();                  /* Gets the counter.  Duh. */
    void     Set_State(void *buffer, uint64_t counter);  /* Resets the state to a saved place. */

  protected:
    std::mt19937 gen; 
    std::uniform_int_distribution<unsigned int> distrib;
    uint64_t Counter;
    bool Use_Second_Normal = false;
    double Second_Normal;
};

inline double MOA::Random_Double() {
    uint32_t result;
    do {
      result = Random_32();
    } while (result == 0xffffffffU);
    return (double)result / (double)(0xffffffffU);
}

inline double MOA::Random_DoubleI() {
    uint32_t result;
    result = Random_32();
    return (double)result / (double)(0xffffffffU);
}

inline double MOA::Random_Normal(double mean, double stddev)
{
  double u, v, s;

  if (Use_Second_Normal) {
    Use_Second_Normal = false;
    return Second_Normal * stddev + mean;
  }

  do {
    u = Random_DoubleI() * 2.0 - 1.0;
    v = Random_DoubleI() * 2.0 - 1.0;
    s = u*u + v*v;
  } while (s >= 1.0 || s == 0.0);

  s = sqrt(-2.0 * log(s) / s);
  Second_Normal = v * s;
  Use_Second_Normal = true;
  return u * s * stddev + mean;
}

inline uint32_t MOA::Random_32() 
{
  uint32_t rv;

  rv = distrib(gen);
  return rv;
}

inline int MOA::Random_Integer() {
  uint32_t r;

  r = Random_32();
  r &= 0x7fffffff;
  return (int) r;
}

inline uint64_t MOA::Random_64() {
  uint64_t sum;

  sum = Random_32();
  sum <<= 32;
  sum |= Random_32();
  return sum;
}

inline void MOA::Random_128(uint64_t *x) {
  x[0] = Random_64();
  x[1] = Random_64();
  return;
}

inline uint32_t MOA::Random_W(int w, int zero_ok)
{
  uint32_t b;

  do {
    b = Random_32();
    if (w == 31) b &= 0x7fffffff;
    if (w < 31)  b %= (1 << w);
  } while (!zero_ok && b == 0);
  return b;
}

/* JSP:  If you give 0 for the seed, then it constructs the seed from:

   - The current time in seconds, xor'd with:
   - The low 12 bits of microseconds, shifted 20 bits left, xor'd with:
   - The remaining 8 bits of microseconds, where they are.
 
   (Since microseconds are < 1000000, it fits into 20 bits).

   So: in hex, if microseconds are ABCDE, and seconds are STUVWXYZ, then
     the seed is:

           STUVWXYZ
       XOR CDEAB000
       ____________
 */

inline uint32_t MOA::Seed_From_Time() 
{
  struct timeval tv;
  uint32_t rv;

  gettimeofday(&tv, NULL);
  rv = (uint32_t) tv.tv_sec;
  rv ^= ((tv.tv_usec & 0xfff) << 20);
  rv ^= (tv.tv_usec & 0xff000);
  return rv;
}

inline void MOA::Seed_XOR(uint32_t seed, uint32_t hash) {
  int i;
  uint32_t s;

  s = (seed == 0) ? Seed_From_Time() : seed;
  s ^= hash;

  gen.seed(s);
  Counter = 0;
  for (i=0; i<19; i++) Random_32();
  Use_Second_Normal = false;
}

inline void MOA::Seed(uint32_t seed, const std::string &name) {
  uint32_t hash;

  hash = MOA::Hash(name);
  Seed_XOR(seed, hash);
}

inline uint32_t MOA::Hash(const std::string &s)
{
  size_t i;
  uint32_t h;

  h = 5381;
  for (i = 0; i < s.size(); i++) h = (h << 5) + h + s[i];
  return h;
}

inline void MOA::Fill_Random_Region (void *reg, int size)
{
  uint32_t *r32;
  uint8_t *r8;
  int i;

  r32 = (uint32_t *) reg;
  r8 = (uint8_t *) reg;
  for (i = 0; i < size/4; i++) r32[i] = Random_32();
  for (i *= 4; i < size; i++) r8[i] = (uint8_t) Random_W(8, 1);
}

inline void MOA::Get_State(void *buffer)
{
  bzero(buffer, sizeof(uint32_t) * 5);
  Use_Second_Normal = false;
}

inline uint64_t MOA::Get_Counter()
{
  return Counter;
}

inline void MOA::Set_State(void *buffer, uint64_t counter)
{
  (void) buffer;
  Counter = 0;
  while (Counter < counter) (void) Random_32();
  Use_Second_Normal = false;
}

}
