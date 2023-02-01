#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>
#include <chrono>

#include <string>

using namespace std::chrono;

#include "TestUtilities.h"

#include "memo.h"
#include "spiceql.h"

#include <spdlog/spdlog.h>

using namespace SpiceQL;
using namespace std;

TEST(UtilTests, testHashCollisions) {
  std::string s1 = "/isisdata/mro/kernels/ck";
  std::string s2 = "/isisdata/mro/kernels/spk"; 

  size_t seed1 = 0;
  size_t seed2 = 0; 
  seed1 = memoization::_hash_combine(seed1, s1);
  seed2 = memoization::_hash_combine(seed2, s2);
  spdlog::debug("seed1 {}", seed1);
  spdlog::debug("seed2 {}", seed2);
 
  EXPECT_NE(seed1, seed2);

  seed1 = 0;
  seed2 = 0;

  seed1 = memoization::hash_combine(seed1, s1, false);
  seed2 = memoization::hash_combine(seed2, s1, true);
  spdlog::debug("seed1 {}", seed1);
  spdlog::debug("seed2 {}", seed2);

  EXPECT_NE(seed1, seed2);
}