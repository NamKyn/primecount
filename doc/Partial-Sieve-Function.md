# The partial sieve function 

The partial sieve function phi(x, a) counts the numbers ≤ x that are not divisible by any of the
first a primes. This function is sometimes also named "Legendre's sum" after the French mathematician
Adrien-Marie Legendre who first studied it in the 19th century [[1]](#References). The
partial sieve function is at the heart of all combinatorial prime counting
algorithms. In fact Legendre's prime counting function pi(x) = pi(√x) + phi(x, pi(√x)) - 1 can
be computed using solely the partial sieve function, whereas the other more advanced combinatorial
prime counting algorithms require evaluating the partial sieve function as well as other functions.

In the advanced combinatorial prime counting algorithms such as the Lagarias-Miller-Odlyzko algorithm
[[3]](#References) and the Deléglise-Rivat algorithm [[4]](#References) the partial sieve function is
mainly used as an auxiliary function during initialization. Therefore the overall performance of these
algorithms does not meaningfully depend upon the execution speed of the partial sieve function.

However, with the advent of multi-core CPUs, the partial sieve function has found another important
use case within the combinatorial prime counting algorithms. For software programs to scale well on
multi-core CPUs it is crucial that the individual worker threads are independent from each other, so
that the threads have exclusive access to the CPU's resources and in order to prevent that a thread has
to wait idle for data from another thread. In 2002 Xavier Gourdon [[5]](#References)
devised a modification to the hard special leaves algorithm so that the computation can be slit up into
independent chunks. This modification relies on the partial sieve function for generating a lookup
table of phi(x, i) results for i ∈ [0, a]. Hence now the partial sieve function's performance
has become critical for parallel implementations of the combinatorial prime counting algorithms.
The [Generate phi(x, i) lookup table](https://github.com/kimwalisch/primecount/blob/master/doc/Partial-Sieve-Function.md#generate-phix-i-lookup-table)
paragraph contains more information.

# Recursive formula

### phi(x, a) = phi(x, a - 1) - phi(x / prime[a], a - 1)

This is the main formula for the computation of the partial sieve function. As mentioned in the
introduction this formula was first described by Legendre in his book "Théorie des nombres"
[[1]](#References). When implemented in a computer program the above recursive
phi(x, a) formula with a = pi(√x) allows to compute Legendre's prime counting function
pi(x) = pi(√x) + phi(x, pi(√x)) - 1 in O(x / log(x)) operations and using O(√x / log(x)) space.
In primecount this formula is implemented in
[phi.cpp](https://github.com/kimwalisch/primecount/blob/master/src/phi.cpp) (together with all
optimizations described in this document).

Tomás Oliveira e Silva's paper [[6]](#References) contains a simple C implementation of this formula:

```C
static void phi(int x, int a, int sign)
{
loop:
    if(a == 0)
        sum += sign * x;
    else if(x < p[a])
        sum += sign;
    else
    {
        --a;
        phi(x / p[a], a, -sign);
        goto loop;
    }
}
```

# Optimizations

There are a large number of known optimizations, that can be used in conjunction with the recursive
phi(x, a) formula and which, when combined, can speed up the computation by many orders for magnitude.
I will now briefly describe all known optimizations, and then further down I will present a new
optimization that I have devised and that has first been implemented in primecount.

### phi(x, a) = (x / pp) * φ(pp) + phi(x % pp, a)

The above formula allows computing phi(x, a) in O(1) for small values of a e.g. for a ≤ 7.
Its use requires initializing a lookup table of size pp = 2 * 3 * ... * prime[a] with
phi(i, a) results for i ∈ [0, pp[. This formula is already present in Lehmer's paper from 1959
[[2]](#References) and is also described in more detail in most of the other combinatorial prime
counting papers. In primecount this formula is implemented in
[PhiTiny.hpp](https://github.com/kimwalisch/primecount/blob/master/include/PhiTiny.hpp) and
the initialization of the lookup table is implemented in
[PhiTiny.cpp](https://github.com/kimwalisch/primecount/blob/master/src/PhiTiny.cpp).

### Stop recursion at c instead of 1

Using the formula from the previous paragraph it is possible to compute phi(x, c) in O(1) for small
values of c e.g. c ≤ 7. Using this formula we can stop recursion at c instead of 1 in the main
[recursive formula](#phix-a--phix-a---1---phix--primea-a---1) and simply increase the sum by phi(x, c).

### Calculate all phi(x / prime[i], i - 1) = 1 upfront in O(1)

Once phi(x / prime[i], i - 1) = 1 occurs in the main
[recursive formula](#phix-a--phix-a---1---phix--primea-a---1) all subsequent phi(x / prime[j], j - 1)
computations with j ∈ ]i, a] will also be 1. Generally phi(x / prime[i], i - 1) = 1 if
(x / prime[i] ≤ prime[i-1]). Hence instead of computing phi(x / prime[j], j - 1) individually for all
j ∈ ]i, a] we can simply increase the sum by a - i.

### if (a ≥ pi(√x)) phi(x, a) = pi(x) - a + 1

This formula also allows computing phi(x, a) in O(1) provided that a is relatively large and x is
relatively small. If (a ≥ pi(√x)) then phi(x, a) counts the number of primes ≤ x, minus the first
a primes, plus the number 1. Using this formula requires the use of a pi(x) lookup table of size x.
In order to reduce the memory usage it is best to use a compressed pi(x) lookup table such as
primecount's [PiTable.hpp](https://github.com/kimwalisch/primecount/blob/master/include/PiTable.hpp).
The use of a lookup table makes this formula unsuitable for computing phi(x, a) for large values
of x due to its excessive memory requirement. However, for large
values of x we can compute the pi(x) part of this formula using a prime counting function
implementation in O(x^(2/3)) or less instead of a lookup table which uses much less memory.

### if (a ≥ pi(3√x) && a < pi(√x)) phi(x, a) = pi(x) + P2(x, a) - a + 1

In the formula above 3√x denotes the 3rd root of x and P2(x, a) is the 2nd partial sieve function
which counts the numbers ≤ x that have exactly 2 prime factors each exceeding the a-th prime.
If (a ≥ pi(4√x) && a < pi(3√x)) then one needs to add the P3(x, a) term i.e.
phi(x, a) = pi(x) + P2(x, a) + P3(x, a) - a + 1. Of all optimizations presented in this section,
this is the only one that is not implemented in primecount as its use would not provide much
benefit to primecount.

# New optimization

Due to the recursive nature of the [main phi(x, a) formula](#phix-a--phix-a---1---phix--primea-a---1)
the same values of phi(i, j) are calculated over and over again, this is especially true for small to
medium values of i and j. The formula phi(x, a) = (x / pp) * φ(pp) + phi(x % pp, a) can be used to
avoid recursion, however it is limited to small values of a ≤ c with c being a small constant e.g.
c = 7. The formula phi(x, a) = pi(x) - a + 1 can also be used to compute phi(x, a) in O(1), however
it is limited to large values of a ≥ pi(√x). Hence there is currently no known optimization for
computing phi(x, a) for medium values of a ∈ ]c, pi(√x)[.

The new optimization that I have devised is a **phi(i, j) cache** for small to medium
values of i and j e.g. i ≤ √x and j ≤ 100. The more phi(i, j) results are cached, the fewer recursive
calls occur in the  [main phi(x, a) formula](#phix-a--phix-a---1---phix--primea-a---1) and the faster
it runs. However, on the other hand we are memory constrained, we cannot cache everything and
ideally our phi(i, j) cache should fit into the CPU's fast cache memory. Hence the main goal for our
cache is to store as many phi(i, j) results as possible using as little memory as possible.

The densest data structure for storing the count of numbers ≤ n that are not divisible by any of the
first a primes that I am aware of is a bit array. If a bit is set, this means the corresponding
number is not divisible by any of the first a primes. The 8 bits of each byte correspond to the offsets
[ 1, 7, 11, 13, 17, 19, 23, 29 ], hence each byte represents an interval of size 30. However to determine
the count of numbers ≤ n that are not divisible by any of the first a primes, we need to iterate over
the bit array and count all set bits that correspond to numbers ≤ n. This means the access time of
our cache would be O(n) which is not great. Therefore we introduce a second array which contains the
count of set bits in the first array below the current index. Using these two arrays we can now count
the numbers ≤ n that are not divisible by any of the first a primes in O(1) operations. Note that the
two arrays may be interleaved which makes our cache slightly more memory efficient. Below is the
corresponding code from primecount:

```C++
  int64_t phi_cache(uint64_t x, uint64_t a) const
  {
    // We devide by 240 instead of 30 here because our arrays
    // use the uint64_t type instead of std::byte.
    uint64_t count = sieve_[a][x / 240].count;
    uint64_t bits = sieve_[a][x / 240].bits;
    uint64_t bitmask = unset_larger_[x % 240];
    return count + popcnt64(bits & bitmask);
  }
```

Before being able to use the phi(x, a) cache it needs to be initialized. The cache can be initialized
using a modified version of the [sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes).
In primecount the cache is lazily initialized during the execution of the
[main phi(x, a) formula](#phix-a--phix-a---1---phix--primea-a---1). Whenever a new phi(x, i) computation
is started we first check whether that result is not yet present in the cache and if x & i meet the caching
criteria. If these conditions apply the cache will be filled up to x & i. In the first part of this algorithm
we unset the bits that correspond to numbers that are divisible by the i-th prime. When sieving has
finished we proceed to the second part of the algorithm where we compute and store the sum of all set bits
below each index. Below is the corresponding code from primecount:

```C++
/// Cache phi(x, i) results with: x <= max_x && i <= min(a, max_a).
/// Eratosthenes-like sieving algorithm that removes the first a primes
/// and their multiples from the sieve array. Additionally this
/// algorithm counts the numbers that are not divisible by any of the
/// first a primes after sieving has completed. After sieving and
/// counting has finished phi(x, a) results can be retrieved from the
/// cache in O(1) using the phi_cache(x, a) method.
///
void init_cache(uint64_t x, uint64_t a)
{
  // Each bit in the sieve array corresponds to an integer that
  // is not divisible by 2, 3 and 5. The 8 bits of each byte
  // correspond to the offsets { 1, 7, 11, 13, 17, 19, 23, 29 }.
  sieve_[3].resize(max_x_size_);

  for (uint64_t i = 4; i <= a; i++)
  {
    // Initalize phi(x, i) with phi(x, i - 1)
    sieve_[i] = sieve_[i - 1];

    // Remove prime[i] and its multiples
    for (uint64_t n = primes_[i]; n <= max_x_; n += primes_[i] * 2)
      sieve_[i][n / 240].bits &= unset_bit_[n % 240];

    // Fill an array with the cumulative 1 bit counts.
    // sieve[i][j] contains the count of numbers < j * 240 that
    // are not divisible by any of the first i primes.
    uint64_t count = 0;
    for (auto& sieve : sieve_[i])
    {
      sieve.count = count;
      count += popcnt64(sieve.bits);
    }
  }
}
```

According to my own benchmarks the cache, as implemented above, speeds up primecount's phi(x, a)
implementation by more than 10x. Based on my empirical tests caching phi(x, a) results for
a ≤ 100 provides the best performance, as mentioned earlier smaller values of x & a are
accessed much more frequently than larger values. I also limit the size of the cache to about 16
megabytes in primecount which is slightly larger than my CPU's L3 cache size. Using an even
larger cache size deteriorates performance especially when using multi-threading.

# Generate phi(x, i) lookup table

In 2002 Xavier Gourdon [[5]](#References) devised a modification to the hard special leaves
algorithm so that the computation can be slit up into independent chunks. This modification is
particularly useful for parallelizing the hard special leaves algorithm, since it avoids the
need for frequent thread synchronization. This modification relies on the partial sieve
function for generating a lookup table of phi(x, i) results for i ∈ [0, a]. The idea of the
algorithm is described very shortly in Gourdon's paper [[5]](#References) and it is also
described in some more detail in Douglas Staple's paper [[7]](#References), however no pseudocode
is provided in both papers.

Computing phi(x, i) individually for all i ∈ [0, a] would be far too slow. However, by taking
advantage of the recursive nature of the main formula phi(x, a) = phi(x, a - 1) - phi(x / prime[a], a - 1),
we can actually generate a lookup table of phi(x, i) results for i ∈ [0, a] in the same
amount of time it takes to compute phi(x, a)! We first compute phi(x, 0), next we compute
phi(x, 1) and reuse the phi(x, 0) result we have computed previously. Then we compute
phi(x, 2) and reuse our previous phi(x, 1) result and so forth. The code below shows how
this algorithm can be implemented using a simple for loop. The ```phi_recursive(x / primes[i], i - 1)```
part needs to be computed using the recursive phi(x, a) formula in conjunction with the
optimizations described in this document.

```C++
int* phi = new int[a + 1];
phi[0] = x;

// Fill an array with phi(x, i) results
for (int i = 1; i <= a; i++)
    phi[i] = phi[i-1] - phi_recursive(x / primes[i], i - 1);
```

# References

1. A. M. Legendre, Théorie des nombres, Third edition, Paris, 1830. Vol. 2, p. 65.
2. D. H. Lehmer, On the exact number of primes less than a given limit, Illinois J. Math. 3 (1959), pp. 381–388.
3. J. C. Lagarias, V. S. Miller, and A. M. Odlyzko, Computing pi(x): The Meissel-Lehmer method, Mathematics of Computation, 44 (1985), pp. 537–560.
4. M. Deleglise and J. Rivat, "Computing pi(x): The Meissel, Lehmer, Lagarias, Miller, Odlyzko Method", Mathematics of Computation, Volume 65, Number 213, 1996, pp 235–245.
5. Xavier Gourdon, Computation of pi(x) : improvements to the Meissel, Lehmer, Lagarias, Miller, Odllyzko, Deléglise and Rivat method, February 15, 2001.
6. Tomás Oliveira e Silva, Computing pi(x): the combinatorial method, Revista do DETUA, vol. 4, no. 6, March 2006, pp. 759-768.
7. Douglas B. Staple, The combinatorial algorithm for computing pi(x), Master of Science Thesis, Dalhousie University Halifax, Nova Scotia, August 2015.
