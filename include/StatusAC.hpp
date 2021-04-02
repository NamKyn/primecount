///
/// @file  StatusAC.hpp
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef STATUSAC_HPP
#define STATUSAC_HPP

#include <int128_t.hpp>

namespace primecount {

class SegmentedPiTable;

class StatusAC
{
public:
  void print(const SegmentedPiTable& segmentedPi);
private:
  bool isPrint(double time);
  double time_ = 0;
  // Only print status if 0.1 seconds have elapsed
  // since last printing the status.
  double is_print_ = 0.1;
};

} // namespace

#endif
