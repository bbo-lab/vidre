#include <vidre/log.hh>

using namespace vidre;

int main(int argc, const char **argv) {
    log::notice() << "A notice...\n";
    return 0;
}
/**@example examples/core/log.cc
 * @brief This file demonstrates logging.
 * 
 */
