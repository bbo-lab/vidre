#include <vidre/settings.hh>
#include <vidre/log.hh>
#include <vidre/exception.hh>

#include <algorithm>

using namespace vidre;

void show(const settings::node &t, int indent = 0) {
    auto info = log::info();

    switch (t.type()) {
        case lua::type::table: {
            if (indent > 0)
                info << log::endl;

            for (int i = 0; i < indent; ++i)
                info << "    ";

            auto keylist = t.keys();
            std::sort(keylist.begin(), keylist.end());

            for (auto key : keylist) {
                info << key << " = ";
                show(t[key], indent + 1);
            }

            return;
        } 
        case lua::type::number:
            info << lua::number(t);
            break;
        case lua::type::string:
            info << std::string(t);
            break;
        default:
            info << "<" << script::g.type_name(t.type()) << "]";
    }
    info << log::endl;
}

int main(int, const char **) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*/core/*");
    log::trace tr;

    std::filesystem::path cfgpath = PROJECT_SOURCE_DIR "/examples/core/settings.lua";

    settings::file cfg;
    cfg.load(cfgpath);

    lua::number version = cfg["version"];
    log::info() << "Config version: " << version << log::endl;

    cfg.save("/dev/stdout");

    return 0;
}

/**@example examples/core/settings.cc
 * @brief Using settings.
 *
 * This file demonstrates how to load configurations and display the contents.
 *
 * */
