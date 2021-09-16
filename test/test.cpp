#include <gtest/gtest.h>
#include <iostream>

GTEST_TEST(UselessTest, test){
    std::cout << "1 == 1 ?" << std::endl;
    EXPECT_EQ(1, 1);
}


int main(int argc, char ** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}