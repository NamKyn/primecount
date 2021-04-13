///
/// @file  LoadBalancerP2.cpp
/// @brief This load balancer assigns work to the threads in the
///        computation of the 2nd partial sieve function.
///        It is used by the P2(x, a) and B(x, y) functions.
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <LoadBalancerP2.hpp>
#include <primecount-internal.hpp>
#include <imath.hpp>
#include <min.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;

namespace primecount {

/// We need to sieve [sqrt(x), sieve_limit[
LoadBalancerP2::LoadBalancerP2(maxint_t x,
                               int64_t sieve_limit,
                               int threads,
                               bool is_print) :
  low_(isqrt(x)),
  sieve_limit_(sieve_limit),
  precision_(get_status_precision(x)),
  is_print_(is_print)
{
  // Ensure that the thread initialization (i.e. the
  // computation of PrimePi(low)) is at most 10%
  // of the entire thread computation.
  int64_t O_primepi = (int64_t) std::pow(sieve_limit, 2.0 / 3.0);
  min_thread_dist_ = O_primepi * 10;
  min_thread_dist_ = max(O_primepi, 1 << 22);

  low_ = min(low_, sieve_limit_);
  int64_t dist = sieve_limit_ - low_;
  thread_dist_ = dist / (threads * 8);
  thread_dist_ = max(min_thread_dist_, thread_dist_);
  threads_ = ideal_num_threads(threads, dist, thread_dist_);
}

int LoadBalancerP2::get_threads() const
{
  return threads_;
}

/// The thread needs to sieve [low, high[
bool LoadBalancerP2::get_work(int64_t& low, int64_t& high)
{
  LockGuard lockGuard(lock_);
  print_status();

  // Calculate the remaining sieving distance
  low_ = min(low_, sieve_limit_);
  int64_t dist = sieve_limit_ - low_;

  if (threads_ == 1)
  {
    if (!is_print_)
      thread_dist_ = dist;
  }
  else
  {
    int64_t max_thread_dist = dist / threads_;

    // Reduce the thread distance near to end to keep all
    // threads busy until the computation finishes.
    if (thread_dist_ > max_thread_dist)
      thread_dist_ = max(min_thread_dist_, max_thread_dist);
  }

  low = low_;
  low_ += thread_dist_;
  low_ = min(low_, sieve_limit_);
  high = low_;

  return low < sieve_limit_;
}

void LoadBalancerP2::print_status()
{
  if (is_print_)
  {
    double time = get_time();
    double old = time_;
    double threshold = 0.1;

    if (old == 0 || (time - old) >= threshold)
    {
      cout << "\rStatus: " << fixed << setprecision(precision_)
           << get_percent(low_, sieve_limit_) << '%' << flush;
    }
  }
}

} // namespace
