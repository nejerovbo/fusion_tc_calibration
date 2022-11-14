# Execute the E84 loopback test
# Note this test requires a very specific configuration with 3 E-84 cards. See JIRA issue for details
./build/src/bin/sample_test E84_loopback -e config/3x_dio8x_eni.xml -s 1000 -i i8254:2 -d 10000
