#include <vidre/util/develop.hh>
#include <vidre/log.hh>

using namespace vidre;

int main(int argc, const char **argv) {
    if (develop::inbuild())
        log::notice() << "Running from inside the build tree.\n";
    else
        log::notice() << "Running outside build tree.\n";

    return 0;
}

/**@example examples/core/develop.cc
 * @brief Development utilities.
 * */
