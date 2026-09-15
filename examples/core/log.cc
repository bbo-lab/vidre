#include <vidre/log.hh>
#include <vidre/error.hh>

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
#if defined(DEBUG)
    {
        log::io::file trf("/tmp/trace.log", log::level::trace);

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
    log::notice() << "done tracing\n";
#endif
}
//! [tracing]

int main(int argc, const char **argv) {
#if not defined(DEBUG)
    log::notice() << "Tracing is disabled (non-DEBUG build).\n";
#endif

    simple_logging();
    block_wise();
    tracing();

    return 0;
}

/**@example examples/core/log.cc
 * @details
 * This file demonstrates logging.
 *
 * This example covers the following topics:
 * - simple log use,
 * - block-wise logging, and,
 * - tracing.
 *
 * */
