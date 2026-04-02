#include <vidre/program.hh>
#include <vidre/log.hh>

#include <iostream>

using namespace vidre;

void version(program::argument::parser &, variant) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

//! [argument-callbacks]
void help(program::argument::parser &p, variant) {
    std::cout << p.help();
    std::exit(0);
}
//! [argument-callbacks]

void log_level(program::argument::parser &, variant value) {
    log::setup(value);
    if (std::string(value) == "help")
        std::exit(0);
}

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    {
        //! [program-options]
        program::options args(  //
            "Example program",  //
            "Copyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany",
            "Example program showing use of the argument parser.",
            {
                {"help", 'h', "Show this usage information.", help},
                {"version", 'V', "Display program version", version},
                {"verbose", 'v', "Increase verbosity"},
                {"log", "Log level (use 'help' for further info).", program::argument::optional, "info", log_level},
                program::argument::parameter{"input", "Input file"},
                program::argument::parameter{"output", "Output file (default: stdout)", "/dev/stdout"},
            });
        //! [program-options]

        program::argument::parser parser(args);
        parser.parse(argc, argv);

        parser.show();
    }

    return 0;
}

/**@example examples/core/program.cc
 * @brief Program invocation and tools example.
 * */
