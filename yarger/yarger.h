//
// Created by ddodd on 8/18/2021.
//

#ifndef ARGPARSER_ARGPARSE_H
#define ARGPARSER_ARGPARSE_H

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <locale>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

using namespace std;

//
// yarger
//
// yarger is yet another argument parser for C++ based applications.
//
// argparse supports git style arguments which means that it supports the notion
// of subcommands.  A subcommand is an argument which is presented immediately after
// the application name.
//  ex: git clone
//      git add
//      git push
//  Here clone, add and push are the subcommands.  I
//

namespace ddi {

    class Option;
    class CommandHelpRunner;

    /**
     * The IArgumentValidator is currently not implemented.
     * This interface will be implemented by users wanting to validate a particular option.  For example, wanting to
     * validate that an integer is a valid integer value, or that a value falls within a specific range.
     */
    class IArgumentValidator {
    public:
        virtual bool validate(Option&) = 0;
        virtual string get_message() = 0;
    };


    /**
     * ICommandRunner interface is used to execute something associated with a Command.
     */
    class ICommandRunner {
    public:
        virtual int run() = 0;
    };

    // Different supported options, ... TBD: actually Int and Float are not supported...
    enum class OptionType { String, Bool, Int, Float, Command };


    class OptionValue {
        string m_value;
    public:
        operator string() const {return m_value;}

        int toInt() {
            return stoi(m_value);
        }
        bool toBool() {
            if (m_value.compare("true")) {
                return true;
            }
            return false;
        }
        float toFloat() {
            return stof(m_value);
        }
        string toString() {
            return m_value;
        }
        
    };




    /**
     * An Option is a representation of an argument from the command line.  The Option has an associated value.
     */
    class Option {
        string m_shortname;
        string m_name;
        string m_description;
        string m_defaultvalue;
        ddi::OptionType m_type;

        // Really should be a template for string, int, bool
        // Would like to use any, but it is not available
        // until we have more access to c++ 17
        string m_value;

    private:
        /***
         * An option can have a command.  For example -h, will
         * list the help for a given command or default.
         */
        ddi::ICommandRunner* m_runner = nullptr;

    public:
        Option(string name) {
            m_name = name;
        }

        ~Option() {
            if (m_runner) {
                delete m_runner;
                m_runner = nullptr;
            }
        }

        Option& type(const ddi::OptionType &type) {
            m_type = type;
            return *this;
        }

        ddi::OptionType get_type() {
            return m_type;
        }

        ddi::ICommandRunner* get_runner() {
            return m_runner;
        }

        Option& value(const string value) {
            m_value = value;
            return *this;
        }

        Option& name(const string &name) {
            m_name = name;
            return *this;
        }

        string get_name() {
            return m_name;
        }

        string get_shortname() {
            return m_shortname;
        }

        string get_description() {
            return m_description;
        }
        string get_value() {
            return m_value;
        }
        
        Option& shortname(const string &shortname) {
            m_shortname = shortname;
            return *this;
        }

        Option& description(const string &description) {
            m_description = description;
            return *this;
        }

        Option& defaultvalue(const string &defaultvalue) {
            m_defaultvalue = defaultvalue;
            m_value = m_defaultvalue;
            return *this;
        }

        Option& command_runner(ddi::ICommandRunner* runner) {
            m_runner = runner;
            return *this;
        }
    };

    /**
     * Command represents a command to be executed.
     * The ArgumentParser has a "default" command.
     * Each sub command of the application is also a Command.
     * Commands have a set of options associated with them as well ad a Runner.
     */
    class Command {
        string                  m_name;
        string                  m_description;
        string                  m_message;
        list<Option*>           m_options;
        map<string, Option*>    m_optionmap;
        bool                    m_ownrunner; // Do we own the pointer to the runner?

        ddi::ICommandRunner* m_runner = nullptr;

    public:

        Command() {}
        Command(string name) {
            m_name = name;
            m_ownrunner = true;
        }

        ~Command() {
            if (m_runner && m_ownrunner) {
                delete m_runner;
                m_runner = nullptr;
            }

            // Clean up our argument
            list<Option*>::iterator itr;
            for ( itr=m_options.begin(); itr != m_options.end(); itr++) {
                delete *itr;
            }
        };

        Option& add_option(string optionname) {
            ddi::Option* newoption = new Option(optionname);
            m_optionmap[optionname] = newoption;
            newoption->type(OptionType::String);
            m_options.push_back(newoption);
            return *m_options.back();
        }

        ddi::Command& name(string name) {
            m_name = name;
            return *this;
        }

        ddi::Command& description(string description) {
            m_description = description;
            return *this;
        }

        list<Option*> get_options() {
            return m_options;
        }

        map<string, Option*>  get_options_map() {
            return m_optionmap;
        }

        string get_name() {
            return m_name;
        }

        string get_message() {
            return m_message;
        }

        string get_description() {
            return m_description;
        }

        Command& message(string message) {
            m_message = message;
            return *this;
        }

        Command& runner(ICommandRunner* runner, bool ownrunner=true) {
            if (m_runner != nullptr && m_ownrunner) {
                delete m_runner;
            }

            m_ownrunner = ownrunner;
            m_runner = runner;
            return *this;
        }

        ddi::ICommandRunner* get_runner() {
            return m_runner;
        }

    };

    /**
     * DefaultRunner
     */
    class DefaultRunner : public ICommandRunner {
    public:
        DefaultRunner(){
        }

        int run() {
            cout << "This command has not defined an acion to take " << endl << endl;
            return 0;
        }
    };


    /**
    *   Help Runner is used to print out the help text for a given command.
    */
    class OptionHelpRunner : public ICommandRunner {
    protected:
        Command* m_command;
        string m_name;
        string m_description;
    public:
        OptionHelpRunner(string name, string description, Command *pCommand) {
            m_command = pCommand;
            m_name = name;
            m_description = description;
        }

        int run() {
            if (m_command->get_message().length() > 0) {
                cout << m_command->get_message() << endl << endl;
            }

            cout << m_name << ": " << m_description << endl;
            cout << "usage: " << m_name << " <command> [<options>] [--help]" << right << endl << endl;

            list<Option*> options = m_command->get_options();
            list<Option*>::iterator it = options.begin();
            while (it != options.end())
            {
                if ((*it)->get_shortname().length() > 0) {
                    cout  << setw(4) << left << "-" + (*it)->get_shortname() + ",";
                }
                cout  << setw(15)   << "--" + (*it)->get_name() << setw(50) <<  (*it)->get_description() << endl;
                it++;
            }
            return 0;
        }
    };

    /**
   *   Help Runner is used to print out the help text for a given command.
   */
    class CommandHelpRunner : public OptionHelpRunner {
        vector<Command*> &m_commands;
    public:
        CommandHelpRunner(string name, string description, vector<Command*> &commands)
        : m_commands(commands), OptionHelpRunner(name, description, nullptr)
        {}

        int run() {
            cout << m_name << ": " << m_description << endl;
            cout << "usage: " << m_name << " <command> [<options>] [--help]" << right << endl << endl;

            cout << "These are the registered commands for " << m_name << endl << endl;
            auto it = m_commands.begin();
            while (it != m_commands.end())
            {
                cout << setw(19) << left << (*it)->get_name()  << setw(50) << left <<  (*it)->get_description() << endl;
                it++;
            }
            return 0;
        }
    };
    /**
     * ArgumentParser is a class to parse arguments.  Argument parser supports the notion of sub commands.  Each
     * sub command can have a set of options.
     */
    class ArgumentParser {

        string m_name;
        string m_description;

        vector<Command*> m_commands;
        map<string,Command*> m_commandmap;


        // The default command is when there are NO sub commands specified
        Command m_default_command;
        Command* m_parsed_command = nullptr;

    protected:

       int parse_args(int argc, char *argv[], Command *pCommand, int arg_start_idx) {

            // Add the default help runner
            m_default_command.runner(new CommandHelpRunner(m_name, m_description, m_commands));

            // Create two look up map to search for the option as we parse the arguments.
            list<Option*> args = pCommand->get_options();
            map<string, Option*> argbyname;
            map<string, Option*> argbyshortname;
            list<Option*>::iterator it;
            for (it= args.begin(); it != args.end(); it++) {
                argbyname.insert(pair<string, Option*>((*it)->get_name(), (*it)));
                argbyshortname.insert(pair<string, Option*>((*it)->get_shortname(), (*it)));
            }

            // Iterate through the arguments and look for options.
            for (int i = arg_start_idx; i < argc; ++i) {

                // Look for a argument
                string arg =  argv[i] ;
                if (arg.find('-') == 0) {
                    int argstart = 1;
                    if  (arg.find('-', 1) == 1) {
                        argstart = 2;
                    }
                    string argname = arg.substr(argstart);
                    auto mapitr = argbyname.find(argname);
                    if (mapitr == argbyname.end()) {
                        mapitr = argbyshortname.find(argname);

                        // Option not found
                        if (mapitr == argbyshortname.end()) {
                            ostringstream  message;
                            m_parsed_command->runner(m_default_command.get_runner(), false);
                            message << "Error: the option <" << argname << "> is not registered" ;
                            pCommand->message(message.str());
                            pCommand->runner( new OptionHelpRunner(m_name, m_description, pCommand));
                            return 1;
                        }
                    }

                    string nextarg;
                    if (i+1 < argc) {
                        nextarg = argv[i+1];
                    }

                    Option* option = (*mapitr).second;
                    ddi::OptionType type = option->get_type();
                    switch (type) {
                        case OptionType::Command:
                            pCommand->runner(option->get_runner(), false);
                            return 0;
                        case OptionType::String:
                            option->value(nextarg) ;
                            i++;
                            break;
                        case OptionType::Bool:
                            option->value("true") ;
                            if (nextarg.find("false") == 0 ||
                                nextarg.find("False") == 0 ) {
                                option->value("false") ;
                                i++;
                            }
                            break;
                        case OptionType::Float: break;
                        case OptionType::Int :
//                            IArgumentValidator* validator = option->get_validator();
//                            if (!validator->validate(option)) {
//                                return 1;
//                            }
                            break;
                        default:
                            break;
                    }
                }
                cout << argv[i]  << endl;
            }
            return 0;
        }

    public:

        ArgumentParser(string name, string description) {
            m_name = name;
            m_description = description;

            // Initialize the default command.
            m_default_command.name(m_name);
            m_default_command.add_option("help").
                    shortname("h").
                    description("Show help").type(OptionType::Command).
                    command_runner(new CommandHelpRunner(m_name,
                                                            m_description,
                                                                  m_commands));
        }

        /**
         * Adds a new command to the argument parser.
         * Add to the vector so we can maintain order of addition.
         * Add to map so that we can have fast look up.
         * Add the "help" option by default, so we can show help for the newly created command.
         * @param command
         * @return
         */
        ArgumentParser& command(Command* command) {
            m_commands.push_back(command);
            Command* cmd = m_commands.back();
            m_commandmap[command->get_name()] = cmd;
            // Add the help command to the user defined sub command.
            cmd->add_option("help").
                shortname("h").
                description("Show help").type(OptionType::Command).
                command_runner(new OptionHelpRunner(m_name, m_description, cmd));
            return *this;
        }

        Command& get_command() {
            return *m_parsed_command;
        }

        ArgumentParser& name(const string &name) {
            m_name = name;
            return *this;
        }
        ArgumentParser& description(const string &description) {
            m_description = description;
            return *this;
        }

        Option& add_option(string optionname) {
            return m_default_command.add_option(optionname);
        }

        bool parse(int argc, char* argv[]) {

            // No arguments given
            // TBD: This needs to be looked at.  The assumption is that the application requires some arguments which is incorrect.
            if (argc == 1) {
                m_parsed_command = &m_default_command;
                m_parsed_command->runner(new CommandHelpRunner(m_name, m_description, m_commands));
                return 1;
            }

            // Find the comamnd.  Look at the first arg to see if it is a sub
            // command.
            int start_parse_idx = 0;
            if (argc > 1) {
                string firstarg =  argv[1] ;
                if (firstarg.find('-') == 0) {
                    // use the default command
                    m_parsed_command = &m_default_command;
                } else {
                    // sub command found
                    start_parse_idx = 2;  // start parsing after the subcommand
                    if (m_commandmap.find(firstarg) != m_commandmap.end()) {
                        m_parsed_command = m_commandmap[firstarg];
                        m_parsed_command->runner(new DefaultRunner());
                    } else {
                        // unregistered subcommand
                        // use default command
                        m_parsed_command = &m_default_command;
                        ostringstream  message;
                        message << "Error: the sub command " << firstarg << " is not registered" << endl << endl;
                        m_parsed_command->message(message.str());
                        m_parsed_command->runner(m_default_command.get_runner(), false);
                        return 1;
                    }
                }
            }
            // parse options
            return parse_args(argc, argv, m_parsed_command, start_parse_idx);
        }
    };

}

#endif //ARGPARSER_ARGPARSE_H
