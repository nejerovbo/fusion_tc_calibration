#include "gtest/gtest.h"
// Pretend vector is another class instead of STL for this POC
#include <vector>
#include <tuple>

using namespace std;

//////////////////////////////////////////////////////////
// Test Fixture with vector STL class composition
// These are the Test types we inherit from Gtest
// ::TestWithParam<int>
// ::Test
// ::TestWithParam<std::tuple<int, bool>>
// ::TestWithParam<std::tuple<int, int>>


// Normal Fixture
class VectorTest : public ::testing::Test
{
public:
    // Class composition
    vector<int> m_vector;

    virtual void SetUp()
    {
        m_vector.push_back(1);
        m_vector.push_back(2);
    }

    virtual void TearDown()
    {
    }
};

// Single Parameter Fixture
class VectorTest2 : public ::testing::TestWithParam<int>
{
public:
    // Class composition
    vector<int> m_vector;

    virtual void SetUp()
    {
        m_vector.push_back(1);
        m_vector.push_back(2);
    }

    virtual void TearDown()
    {
    }
};

// Multiple Parameter Fixture
class VectorTest3 : public ::testing::TestWithParam<std::tuple<int, bool>>
{
public:
    // Class composition
    vector<int> m_vector;

    virtual void SetUp()
    {
        m_vector.push_back(1);
        m_vector.push_back(2);
    }

    virtual void TearDown()
    {
    }
};
///////////////////////////////////////////////////

// Multiple Parameter Test
TEST_P(VectorTest3, DISABLED_MultipleParameterTest)
{
    int index = std::get<0>(GetParam());
    int expected = std::get<1>(GetParam());

    // Calling a property of a class to see if it matches digit
    EXPECT_EQ(m_vector[index], expected);
}

INSTANTIATE_TEST_SUITE_P(
    RandomName,
    VectorTest3,
    ::testing::Values(
        // index, digit
        std::make_tuple(0, 1),
        std::make_tuple(1, 2),
        std::make_tuple(0, 1),
        std::make_tuple(0, 2),
        std::make_tuple(1, 1)));

// Single Parameter Test
TEST_P(VectorTest2, DISABLED_OneParameterTest)
{
    int digit = (int)GetParam();

    // Calling a property of a class to see if it matches digit
    EXPECT_EQ(digit, m_vector[1]);
}

INSTANTIATE_TEST_SUITE_P(
    RandomName,
    VectorTest2,
    ::testing::Values(
        1, 2, 1, 2, 1));

// Testing done through a loop
TEST(VectorTest, DISABLED_LoopTest)
{
    // Pretend we are instantiating an object and getting data
    vector<int> m_vector{1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1};

    int val = 1;
    int i = 0;
    for (;;)
    {
        EXPECT_EQ(m_vector[i], val) << "We got a wrong value of " << i << " instead of " << val << endl;
        m_vector.push_back(1);
        m_vector.pop_back();
        i++;
        // if (i == 100)
        // 	break;
    }
}

TEST_F(VectorTest, DISABLED_testElementZeroIsOne)
{
    EXPECT_EQ(1, m_vector[0]);
}

TEST_F(VectorTest, DISABLED_testElementOneIsTwo)
{
    EXPECT_EQ(2, m_vector[1]);
}

TEST_F(VectorTest, DISABLED_testSizeIsTwo)
{
    unsigned int three = 3;

    EXPECT_EQ((unsigned int)2, m_vector.size());
    EXPECT_EQ(three, m_vector.size()) << "The size of the vector is not size" << three << endl;
}
