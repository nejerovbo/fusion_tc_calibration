# Creating Test Script by Jesse
# Includes loops, random shuffles, wildcarding, disabling, enabling
# and creating report files

# Format of Gtest Naming conventions
# TestSuite.TestCases

# Tests with a function call
test_suite_1="SampleTest"
test_suite_2="IntAddition" 

# Testing cout and differest tests
test_suite_3="ExampleTests"

# Simple Fixture Test
test_suite_4="VectorTest"

# Single then multiple parameter test
test_suite_5="VectorTest2"
test_suite_6="VectorTest3"

# DVT Test Suite
test_suite_dvt="DvtTest"

# Number of times you want your tests to run
test_runs=100

# Lists all current tests suites and their test cases
./build/tests/bin/sample_test --gtest_list_tests

# # Runs all the test but they are disabled
# ./build/tests/bin/sample_test --gtest_filter=*

# # Runs all tests even disabled
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests 

# # Runs all the test from specific test suite
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_filter=$test_suite_1.*

# # Runs test from two test suites (OR)
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_filter=$test_suite_2.*:$test_suite_3.*

# Runs test_suite_4 which has a test that has an infinite loop
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_filter=$test_suite_4.*

# Runs test_suite_4 which has a test that has an infinite loop but this time we take out that test with an infinite loop
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_filter=$test_suite_4.*-$test_suite_4.DISABLED_LoopTest

# Runs the same type of test as previous but this time to 100 times
# ./build/tests/bin/sample_test --gtest_repeat=$test_runs --gtest_also_run_disabled_tests --gtest_filter=.*-$test_suite_4.DISABLED_LoopTest

# Runs shuffle using random seed
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_shuffle --gtest_filter=$test_suite_1.*:$test_suite_2.*:$test_suite_3.*

# Runs shuffle using random seed 100 times
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_shuffle --gtest_filter=$test_suite_1.*:$test_suite_2.*:$test_suite_3.*

# Runs shuffle using random seed 100 times
#./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_shuffle --gtest_repeat=$test_runs --gtest_filter=$test_suite_1.*:$test_suite_2.*:$test_suite_3.*

# Runs the DVT test suite
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_filter=$test_suite_dvt.*

# Creates a XML Report of the test ran above
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_shuffle --gtest_filter=$test_suite_1.*:$test_suite_2.*:$test_suite_3.* --gtest_output=xml:./report/report.xml

# Creates a JSON Report of the test ran above
# ./build/tests/bin/sample_test --gtest_also_run_disabled_tests --gtest_shuffle --gtest_filter=$test_suite_1.*:$test_suite_2.*:$test_suite_3.* --gtest_output=json:./report/report.json

# Show both reports made in the report directory
# cat report/report.json && cat report/report.xml

# Remove the report directories made
# rm -rf report
