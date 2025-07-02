#include <vidre/plugin.hh>

#include <vidre/util/develop.hh>

using namespace vidre;

static std::string_view vidre_plugin_dir = VIDRE_INSTALL_PLUGINDIR;
// static std::string_view vidre_data_dir

int main(int argc, const char **argv) {
    if (develop::inbuild()) {
        vidre_plugin_dir = Vidre_CURRENT_BINARY_DIR;
    }
    
    return 0;
}

/**@example examples/core/plugins.cc
 * @brief FIXME
 *
 * */
