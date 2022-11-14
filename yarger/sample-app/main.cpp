//
// Created by ddodd on 8/23/2021.
//

#include "yarger.h"


int main(int argc, char *argv[]) {


    ddi::ArgumentParser parser("FAT", "Fusion Automated Tester");

    // DVT_ENI_FILE

    ddi::Command dvt("dvt");
     dvt.add_option("eni-file").shortname("e").
        description("eni input file").defaultvalue("fubar");
     dvt.add_option("refresh-rate").shortname("r").
        description("Refresh rate");
     dvt.add_option("nic").shortname("i").
        description("Network interface to use");
     dvt.add_option("display-rate").shortname("d").
        description("Display rate in cyclic frames");
     dvt.description("Run the DVT tests");
    parser.command(&dvt);

    int retval = parser.parse(argc, argv);
    ddi::Command& command = parser.get_command();
    string command_name = command.get_name();

    if (retval == 0) {
        if (!command_name.compare("dvt")) {
            std::map<string, ddi::Option*> options = command.get_options_map();
            string eni_file     = options["eni-file"]->get_value();
            string refresh_rate = options["refresh-rate"]->get_value();
            string nic          = options["nic"]->get_value();
            string display_rate = options["display-rate"]->get_value();

            cout << "eni_file: " << eni_file << endl;
            cout << "refresh_rate: " << refresh_rate << endl;
            cout << "nic: " << nic << endl;
            cout << "display_rate: " << display_rate << endl;

            // Create and environment for dvt tests
            command.get_runner()->run();

        }
        else {
            command.get_runner()->run();
        }
    } else {
        cout << "hello" << endl;
        command.get_runner()->run();
    }

}

