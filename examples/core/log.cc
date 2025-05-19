#include <vidre/log.hh>

using namespace vidre;

//! [simple-log]
void simple_logging() {
    log::warning() << "A warning...\n";
    log::notice() << "...and a notice" << log::endl;
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

    tr << "entering level 3\n";
    tr << "leaving level 3\n";
}

void level2() {
    log::trace tr;
    tr << "entering level 2\n";

    level3();

    tr << "leaving level 2\n";
}

void level1() {
    log::trace tr;
    tr << "entering level 1\n";

    level2();

    tr << "leaving level 1\n";
}

void tracing() {
    // set default streamer threshold to "trace"
    log::io::stream::threshold(0, log::level::trace);

    // set wildcard pattern for what to trace
    log::trace::pattern("*/core/*");
    
    // trace...
    log::debug() << "starting to trace\n";
    log::trace tr;
    tr << "tracing 3 levels\n";

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

/**@example examples/core/log.cc
 * @brief This file demonstrates logging.
 *
 * This file demonstrates logging covering the following topics:
 * - simple log use,
 * - block-wise logging,
 * - etc.
 *
 */
