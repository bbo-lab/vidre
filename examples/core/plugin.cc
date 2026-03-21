#include <vidre/plugin.hh>
#include <vidre/log.hh>
#include <vidre/util/develop.hh>

using namespace vidre;

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");
    log::trace tr;

    tr << "Exploring plugins...\n";

    // find plugins
    plugin::hub::find();

    tr << "Done.\n";
    return 0;
}

/**@example examples/core/plugins.cc
 * @brief FIXME
 *
 * */
