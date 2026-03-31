#include <vidre/program.hh>
#include <vidre/log.hh>

#include <iostream>

using namespace vidre;

void version(program::argument::parser &, const std::string &, variant) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

void help(program::argument::parser &, const std::string &, variant) {
    // p.help();
    std::exit(0);
}

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    program::options args(  //
        "Example program\nCopyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany",
        "Example program showing use of the argument parser.",
        {
            {"help", 'h', "Show this usage information.", help},
            {"version", 'V', "Display program version", version},
            {"verbose", 'v', "Increase verbosity"},
            program::argument::parameter{"input", "Input file"},
            program::argument::parameter{"output", "Output file (default: stdout)", "/dev/stdout"},
        });

    // args.parse(argc, argv);
    // args.show();

    program::parse(args, argc, argv);

    return 0;
}

/**@example examples/core/app.cc
 * @brief Application tools.
 * */
