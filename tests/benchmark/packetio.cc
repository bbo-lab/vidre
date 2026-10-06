#include <vidre/util/chrono.hh>
#include <vidre/util/debug.hh>
#include <vidre/cpp/format.hh>
#include <vidre/program.hh>
#include <vidre/log.hh>
#include <vidre/io.hh>

#include <iostream>

using namespace vidre;

void version(program::argument::parser &, variant) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

void help(program::argument::parser &p, variant) {
    std::cout << p.help();
    std::exit(0);
}

void log_level(program::argument::parser &, variant value) {
    log::setup(value);
    if (std::string(value) == "help")
        std::exit(0);
}

int main(int argc, const char **argv) {
    try {
        //! [program-options]
        program::options args(  //
            "Packet IO Benchmark",  //
            "Copyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany",
            "This program benchmarks packet IO performance.",
            {
                {"help", 'h', "Show this usage information.", help},
                {"version", 'V', "Display program version", version},
                {"verbose", 'v', "Increase verbosity"},
                {"log", "Log level (use 'help' for further info).", program::argument::optional, "info", log_level},
                {"list", 'l', "List available tests."},
            });
        //! [program-options]

        program::argument::parser parser(args);
        parser.parse(argc, argv);

        parser.show();
    } catch (exception &e) {
        std::cerr << error::diagnostic(e).dump() << std::endl;
    }

    return 0;
}
