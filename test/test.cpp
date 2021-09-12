#include <gtest/gtest.h>
#include <iostream>

GTEST_TEST(UselessTest, test){
    std::cout << "1 == 1 ?" << std::endl;
    EXPECT_EQ(1, 1);
}