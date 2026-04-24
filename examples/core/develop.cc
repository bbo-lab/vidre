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
 * @details
 * Development utilities.
 *  
 * @note The function `develop::inbuild()` is only available when the VIDRE_DEVELOP 
 *       setting is enabled in CMake. For release builds, it will always return @c false.
 * 
 * */
