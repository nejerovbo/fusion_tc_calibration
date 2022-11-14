#include "gtest/gtest.h"
#include "../include/functions.h"

using namespace std;


TEST(IntAddition, DISABLED_Negative)
{
    EXPECT_EQ(-5, int_addition(-2, -3)) << "This will be shown in case it fails" << endl;
    EXPECT_EQ(-3, int_addition(5, -8));
}

TEST(IntAddition, DISABLED_Positive)
{
    EXPECT_EQ(4, int_addition(1, 3));
    EXPECT_EQ(9, int_addition(4, 5));
}

TEST(ExampleTests, DISABLED_DemonstrateGTestMacros)
{
    EXPECT_TRUE(true);
    EXPECT_TRUE(false) << "EXPECT_TRUE failed";
    EXPECT_EQ(true, true);
    ASSERT_EQ(1, 1);
    EXPECT_EQ(1, 0);
    ASSERT_TRUE(false);
    ASSERT_TRUE(true);
    ASSERT_TRUE(true);
    ASSERT_TRUE(true);
}