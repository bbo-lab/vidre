#include <vidre/application.hh>
#include <vidre/log.hh>

#include <iostream>

using namespace vidre;

static int version(arg::parser &) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

static int help(arg::parser &p) {
    return p.help();
}

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    arg::parser args{
        {"help", 'h', help, "Show this usage information."},
        {"version", 'V', version, "Display program version"}
    };

    args.parse(argc, argv);

    return 0;
}

/**@example examples/core/app.cc
 * @brief Application tools.
 * */
