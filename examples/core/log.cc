#include <vidre/log.hh>

using namespace vidre;

#if 0

//! [simple-log]
void simple_logging() {
    log::notice() << "A notice..." << log::endl;
}
//! [simple-log]

//! [block-wise]
void block_wise() {
    log::notice notice;

    notice << "a = " << 1 << log::endl;
    notice << "b = " << 2 << log::endl;
}
//! [block-wise]

//! [tracing]
void level3() {
    log::trace tr;

}

void level2() {
    log::trace tr;

    level3();
}

void level1() {
    log::trace tr;

    level2();

    tr << "done" << log::endl;
}

void tracing() {
    log::trace tr;
    tr << "tracing 3 levels" << log::endl;

    level1();
}
//! [tracing]

int main(int argc, const char **argv) {
#if not defined(DEBUG)
    // log::fatal() does not return but instead terminates the program
    log::fatal() << "This example should be executed with full debugging enabled.\nExiting...\n\n";
#endif

    simple_logging();
    block_wise();
    tracing();

    return 0;
}

#else
int main(int argc, const char **argv) {
    return 0;
}
#endif

/**@example examples/core/log.cc
 * @brief This file demonstrates logging.
 *
 * This file demonstrates logging covering the following topics:
 * - simple log use,
 * - block-wise logging,
 * - etc.
 *
 */
