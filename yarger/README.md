# yarger
# Yet Another Option Parser for C++
This is yeat another arg parser for C++.  There seems to be a ton of different ones.  The decision to write another one
was made because we really wanted support for "sub commands".  Sub commands are similar to how git works where by you 
have multiple sub commands like clone, fetch, push etc...

# Using
Each sub command can have it's own set of options.

The commands and options are easy to configure.
```shell

int main(int argc, char *argv[]) {

    ddi::ArgumentParser parser("sample-app", "test the parser out...!");

    // Create some args
    ddi::Command run("run");
    run.description("This is a test subcommand");
    run.add_option("test").shortname("t").
        description("Used to a test that we are just testing");
    run.add_option("volume").
        shortname("v").type(ddi::OptionType::Bool);
    run.add_option("build").
        shortname("b").description("Set the run in Debug mode");
    parser.command(&run);

    ddi::Command& command = parser.get_command();
    command.get_runner()->run();

}
```

# Building
```shell
rm -rf build
mkdir build
conan install . -s build_type=Debug  -if build

cmake -B build -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Debug -A x64
cmake --build build  --config Debug
```


