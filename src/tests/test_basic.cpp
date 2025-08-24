#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

class BasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }
    
    void TearDown() override {
        // Common cleanup for all tests
    }
};

// Basic functionality tests
TEST_F(BasicTest, StringOperations) {
    std::string test_string = "Hello, World!";
    EXPECT_EQ(test_string.length(), 13);
    EXPECT_EQ(test_string.substr(0, 5), "Hello");
    EXPECT_TRUE(test_string.find("World") != std::string::npos);
}

TEST_F(BasicTest, VectorOperations) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[4], 5);
    
    numbers.push_back(6);
    EXPECT_EQ(numbers.size(), 6);
    EXPECT_EQ(numbers[5], 6);
}

TEST_F(BasicTest, MathematicalOperations) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_EQ(10 - 5, 5);
    EXPECT_EQ(3 * 4, 12);
    EXPECT_EQ(15 / 3, 5);
    EXPECT_EQ(7 % 3, 1);
}

TEST_F(BasicTest, BooleanOperations) {
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(true && true, true);
    EXPECT_EQ(true || false, true);
    EXPECT_EQ(!false, true);
}

TEST_F(BasicTest, ComparisonOperations) {
    EXPECT_EQ(5, 5);
    EXPECT_NE(5, 6);
    EXPECT_LT(3, 7);
    EXPECT_LE(5, 5);
    EXPECT_GT(10, 3);
    EXPECT_GE(8, 8);
}

// Test fixture example
class MathTest : public BasicTest {
protected:
    int a = 10;
    int b = 5;
};

TEST_F(MathTest, Addition) {
    EXPECT_EQ(a + b, 15);
}

TEST_F(MathTest, Subtraction) {
    EXPECT_EQ(a - b, 5);
}

TEST_F(MathTest, Multiplication) {
    EXPECT_EQ(a * b, 50);
}

TEST_F(MathTest, Division) {
    EXPECT_EQ(a / b, 2);
}

// Parameterized test example
class ParameterizedTest : public ::testing::TestWithParam<std::tuple<int, int, int>> {
};

TEST_P(ParameterizedTest, Addition) {
    auto params = GetParam();
    int a = std::get<0>(params);
    int b = std::get<1>(params);
    int expected = std::get<2>(params);
    
    EXPECT_EQ(a + b, expected);
}

INSTANTIATE_TEST_SUITE_P(
    AdditionTests,
    ParameterizedTest,
    ::testing::Values(
        std::make_tuple(1, 1, 2),
        std::make_tuple(2, 3, 5),
        std::make_tuple(10, 20, 30),
        std::make_tuple(-1, 1, 0)
    )
);

// Mock example (if needed)
class MockCalculator {
public:
    MOCK_METHOD(int, add, (int a, int b), ());
    MOCK_METHOD(int, multiply, (int a, int b), ());
};

TEST_F(BasicTest, MockExample) {
    MockCalculator calc;
    
    EXPECT_CALL(calc, add(2, 3))
        .WillOnce(::testing::Return(5));
    
    EXPECT_CALL(calc, multiply(4, 5))
        .WillOnce(::testing::Return(20));
    
    EXPECT_EQ(calc.add(2, 3), 5);
    EXPECT_EQ(calc.multiply(4, 5), 20);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
