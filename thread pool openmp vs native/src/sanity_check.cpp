#include <gtest/gtest.h>
#include "MonteCarloPi.h"


TEST(MonteCarloPi, ThreadPoolResultSanity)
{
  double result = getMonteCarloPi_ThreadPool(100000, 42);
  EXPECT_NEAR(result, 3.14, 0.1);
}

TEST(ThreadPoolTest, OpenMPResultSanity)
{
  double result = getMonteCarloPi_OpenMP(100000, 42);
  EXPECT_NEAR(result, 3.14, 0.1);
}
