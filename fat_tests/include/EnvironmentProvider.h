//
// Created by ddodd on 8/27/2021.
//

#ifndef FATPARENT_ENVIRONMENTPROVIDER_H
#define FATPARENT_ENVIRONMENTPROVIDER_H
#include "gtest/gtest.h"
#include "yarger.h"
#include "AcontisEnvironment.h"

using namespace ddi;

/**
 * Adds a default set of options for DDI
 * @param command
 */
void add_default_options(ddi::Command& command);


/**
 * DDI specific command for common functionality and options
 */

class DDICommand : public ddi::Command {
protected:
    ddi::ArgumentParser* m_parser;

public:
    // Abstract method which returns the regex which is passed to gtest_filter
    virtual string GetTestFilter() = 0;
    // Return the description for the DDI test
    virtual string GetDescription() = 0;

public:
    DDICommand(string name) : ddi::Command(name) {
    }

    virtual void Initialize(ddi::ArgumentParser* parser) {
        m_parser = parser;
        add_default_options(*this);
        this->add_option("testfilter").defaultvalue(GetTestFilter()).
            description("Sets the filter for the test to be used");
        this->description(GetDescription());
        m_parser->command(this);
    }

    virtual ::testing::Environment* GetEnvironment() {
        return new AcontisEnvironment(m_parser->get_command().get_options_map());
    }
};

/**
 * Provides a google test environment
 */
class EnvironmentProvider {
public:
    virtual ::testing::Environment* GetEnvironment() = 0;
};


/**
 * The DDI Environment provides a google test environment
 * based on the arguments passed in.
 */
class DDIEnvironmentProvider : public EnvironmentProvider {

    int m_argc;
    char **m_argv;

public:
    DDIEnvironmentProvider(int argc, char* argv[]);

    virtual ::testing::Environment* GetEnvironment();
};

#endif //FATPARENT_ENVIRONMENTPROVIDER_H



