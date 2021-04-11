///
/// @file   P2_xa.cpp
/// @brief  Test the 2nd partial sieve function P2(x, a)
///         that counts the numbers <= x that have exactly
///         2 prime factors each exceeding the a-th prime.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primecount-internal.hpp>
#include <generate.hpp>
#include <imath.hpp>

#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <random>

using namespace std;
using namespace primecount;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

int main()
{
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<int> dist(50000, 70000);

  int threads = 1;
  int64_t x = dist(gen);
  auto primes = generate_primes<int64_t>(x);

  for (int a = 1; primes[a] <= isqrt(x); a++)
  {
    int64_t p2 = 0;

    for (size_t b = a + 1; b < primes.size(); b++)
      for (size_t c = b; c < primes.size(); c++)
        if (primes[b] * primes[c] <= x)
          p2++;

    cout << "P2(" << x << ", " << a << ") = " << p2;
    check(p2 == P2(x, primes[a], threads));
  }

  cout << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
