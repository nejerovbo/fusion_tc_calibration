#include "gtest/gtest.h"
#include "yarger.h"


TEST(SuiteName, TestName) {
    
}

TEST(SimpleTest, test_simple_command) {

    ddi::ArgumentParser parser("fat-and-simple", "Fusion Automated Tester");

    // Create some args
    int argc = 7;
    char* argv[] = {"cpptest.exe", "run", "-t", "test", "-v", "--build", "Debug"};
    ddi::Command run("run");
    run.add_option("test").shortname("t").
            description("Used to a test that we are just testing");
    run.add_option("volume").
            shortname("v").type(ddi::OptionType::Bool);
    run.add_option("build").
            shortname("b").description("Set the run in Debug mode");
    parser.command(&run).description("This is a test subcommand");
    int retval = parser.parse(argc, argv);
    EXPECT_EQ (0,  retval);

    ddi::Command& command = parser.get_command();
    list<ddi::Option*> options = command.get_options();
    map<string, ddi::Option*> argbyname;

    list< ddi::Option*>::iterator it;
    for (it= options.begin(); it != options.end(); it++) {
        argbyname.insert(pair<string,  ddi::Option*>((*it)->get_name(), (*it)));
    }

    std::map<string, ddi::Option*>::iterator optionitr = argbyname.find("test");
    EXPECT_NE(optionitr, argbyname.end());

}

TEST(SimpleTest, unregistered_command) {

    ddi::ArgumentParser parser("fat-default", "Fusion Automated Tester");

    // Create some args
    int argc = 7;
    char* argv[] = {"cpptest.exe", "run", "-t", "test", "-v", "--build", "Debug"};
    parser.add_option("test").shortname("t").
                    description("Used to a test that we are just testing");
    parser.add_option("volume").
    shortname("v").type(ddi::OptionType::Bool);
    parser.add_option("build").
    shortname("b").description("Set the run in Debug mode");
    int retval = parser.parse(argc, argv);

    // we expect the value to return a 1 since no commands were registered
    EXPECT_EQ (1,  retval);
}


TEST(SimpleTest, help_command_default_test) {

    ddi::ArgumentParser parser("fat-help", "Fusion Automated Tester");

    // Create some args
    int argc = 2;
    char* argv[] = {"cpptest.exe", "-h"};
    int retval = parser.parse(argc, argv);

    // Expecting the value of 1 because we are not running anything
    EXPECT_EQ (0,  retval);
}


void add_options(ddi::Command& command) {
    
    command.add_option("test").shortname("t").
            description("Used to a test that we are just testing").
            defaultvalue("default");
    command.add_option("volume").
            shortname("v").type(ddi::OptionType::Bool);
    command.add_option("build").
            shortname("b").description("Set the run in Debug mode");
    command.description("this is a test command");
}


TEST(DefaultTest, TestDefaultSet) {

    ddi::ArgumentParser parser("dvt", "Fusion Automated Tester");

    // Create some args
    int argc = 5;
    char* argv[] = {"cpptest.exe", "run", "-v", "--build", "Debug"};
    ddi::Command run("run");
    add_options(run);
    parser.command(&run);

    int retval = parser.parse(argc, argv);
    EXPECT_EQ (0,  retval);

    ddi::Command& command = parser.get_command();
    std::map<string, ddi::Option*> options = command.get_options_map();
   
    ASSERT_EQ("default", options["test"]->get_value());
}


TEST(DefaultTest, TestDefaultOverridden) {

    ddi::ArgumentParser parser("dvt", "Fusion Automated Tester");

    // Create some args
    int argc = 7;
    char* argv[] = {"cpptest.exe", "run", "-t", "test", "-v", "--build", "Debug"};
    ddi::Command run("run");
    add_options(run);
    parser.command(&run);

    int retval = parser.parse(argc, argv);
    EXPECT_EQ (0,  retval);

    ddi::Command& command = parser.get_command();
    std::map<string, ddi::Option*> options = command.get_options_map();
   
    ASSERT_EQ("test", options["test"]->get_value());

}




