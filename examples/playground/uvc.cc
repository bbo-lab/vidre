#include <vidre/uvc/device.hh>
#include <vidre/log.hh>
#include <vidre/program.hh>

#include <iostream>
#include <string>
#include <filesystem>
#include <sstream>

using namespace vidre;
using namespace vidre::video;

// ============================================================================

void program_version(program::argument::parser &, variant) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

void program_help(program::argument::parser &p, variant) {
    std::cout << p.help();
    std::exit(0);
}

void log_level(program::argument::parser &, variant value) {
    log::setup(value);
    if (std::string(value) == "help")
        std::exit(0);
}

void list(program::argument::parser &, variant) {
    uvc::camera::enumerator enumerator;
    auto cameras = enumerator.find();

    int i = 0;
    for (const auto &dev : cameras) {
        uvc::camera *camera = dynamic_cast<uvc::camera *>(dev.get());

        std::cout << std::format(("[{:>2}] Name: {}\n       ID: {}\n"), ++i, camera->name(), camera->id());

        //
        if (log::io::stream::threshold(0) >= log::level::info) {
            std::cout << "     Info:\n";
            
            std::string info = camera->meta().dump(4);
            std::istringstream info_stream(info);
            std::string line;
            while (std::getline(info_stream, line)) {
                std::cout << "           " << line << '\n';
            }
        }
    }

    std::exit(0);
}

int main(int argc, const char **argv) {
    try {
        //! [program-options]
        program::options args(  //
            "UVC playground",   //
            "Copyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany", "UVC driver development playground.",
            {
                {"help", 'h', "Show this usage information.", program_help},
                {"version", 'V', "Display program version", program_version},
                {"verbose", 'v', "Increase verbosity"},
                {"list", "List available cameras"},
                {"log", "Log level (use 'help' for further info).", program::argument::optional, "info", log_level},
                program::argument::parameter{"output", "Output file (default: stdout)", "/dev/stdout"},
            });
        //! [program-options]

        program::argument::parser parser(args);
        parser.parse(argc, argv);

        if (parser.has_unparsed()) {
            std::cerr << "Unrecognized arguments:\n";
            for (const auto &arg : parser.unparsed()) {
                std::cerr << "  " << arg << "\n";
            }
            std::exit(1);
        }

        if (long(parser.get("list")) > 0L) {  // FIXME this is ugly
            list(parser, {});
            std::exit(0);
        }

        // FIXME run camera and write output to file
    } catch (exception &e) {
        std::cerr << error::diagnostic(e).dump() << std::endl;
    }

    return 0;
}
