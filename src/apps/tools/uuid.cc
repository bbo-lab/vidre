#include <vidre/program.hh>
#include <vidre/log.hh>
#include <vidre/util/uuid.hh>

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
    log::io::stream::threshold(0, log::level::notice);
    log::trace::pattern("*");

    try {
        program::options args(  //
            "UUID generator",   //
            "Copyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany", "Create a UUID.",
            {{"help", 'h', "Show this usage information.", help},
             {"version", 'V', "Display program version", version},
             {"log", "Log level (use 'help' for further info).", program::argument::optional, "info", log_level},
             {"type", "UUID type (use 'help' for further info).", program::argument::required, variant::cstring}});

        program::argument::parser parser(args);
        parser.parse(argc, argv);

        parser.show();

        // make UUID
        auto type = parser.get("type");
        std::string type_s = "safe";
        if (not type.empty())
            type_s = std::string(type);

        if (type_s == "help") {
            std::cout << "Available UUID types:\n"
                         "  - safe: Time- and MAC-based UUID with synchronization (throws if synchronization fails, default)\n"
                         "  - time: Time- and MAC-based UUID (may leak information about generation time and machine)\n"
                         "  - random: Random or pseudo-random UUID\n";
        }
        else if (type_s == "safe") {
            std::cout << uuid::makeTimeSafe().tostring() << std::endl;
        }
        else if (type_s == "random") {
            std::cout << uuid::makeRandom().tostring() << std::endl;
        }
        else if (type_s == "time") {
            std::cout << uuid::makeTime().tostring() << std::endl;
        }
        else {
            throw exception::argument() << error::message("Invalid UUID type specified.");
        }

    } catch (exception &e) {
        std::cerr << error::diagnostic(e).dump() << std::endl;
    }

    return 0;
}
