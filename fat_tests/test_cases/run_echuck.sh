# Execute the E84 loopback test
# Note this test requires a very specific configuration with 3 E-84 cards. See JIRA issue for details
./build/src/bin/sample_test echuck-op-mode -e config/echuck_eni.xml -s 10000 -i $1 -d 10000
