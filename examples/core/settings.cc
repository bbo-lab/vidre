#include <vidre/settings.hh>
#include <vidre/log.hh>
#include <vidre/exception.hh>

#include <algorithm>

using namespace vidre;

//! [settings-json]
void setup(const std::string &optstring) {
    settings::json options = settings::json::parse(optstring);
    log::info() << "\nOptions as string (JSON):\n" << options.dump(4) << log::endl;
}

void setup(settings::json options) {
    log::info() << "\nOptions as variant (JSON):\n" << options.dump(4) << log::endl;
    setup(options.dump());
}
//! [settings-json]

int main(int, const char **) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*/core/*");
    log::trace tr;

    //! [settings-file]
    std::filesystem::path cfgpath = PROJECT_SOURCE_DIR "/examples/core/settings.lua";

    settings::file cfg;
    cfg.load(cfgpath);

    lua::number version = cfg["version"];
    log::info() << "Config version: " << version << log::endl;

    log::info() << "File contents (Lua): " << log::endl;
    cfg.save("/dev/stdout");
    //! [settings-file]

    //! [settings-json]
    setup({{"fps", 20},  //
           {"geometry",
            {
                {"x", 0},        //
                {"y", 0},        //
                {"width", 320},  //
                {"height", 200}  //
            }}});
    //! [settings-json]

    return 0;
}

/**@example examples/core/settings.cc
 * @details
 * Using settings.
 *
 * This file demonstrates how to load a configuration file (Lua) and passing
 * options to functions (JSON).
 *
 * */
