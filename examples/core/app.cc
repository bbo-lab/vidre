#include <vidre/application.hh>
#include <vidre/log.hh>

#include <iostream>

using namespace vidre;

static int version(arg::parser &) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

static int help(arg::parser &p) {
    p.help();
    std::exit(0);
}

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    arg::parser args( //
        " - Example program\nCopyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany",
        "Example program showing use of the argument parser.",
        {
        {"help", 'h', help, "Show this usage information."},
        {"version", 'V', version, "Display program version"},
        {"verbose", 'v', "Increase verbosity"},
        {"input", arg::option::positional_argument, "Input file"},
        {"output", arg::option::positional_argument, "/dev/stdout", "Output file (default: stdout)"},
    });

    args.parse(argc, argv);
    args.show();

    return 0;
}

/**@example examples/core/app.cc
 * @brief Application tools.
 * */
