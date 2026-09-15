#include <vidre/plugin/information.hh>

void init() {
    // empty
}

// clang-format off
VIDRE_MODULE(
    .name = "TestNull",
    .version = "0.0",
    .organization = {
        .name = "MPINB",
        .domain = "de.mpg.mpinb"
    },
    .brief = "Example module that does nothing.",
    .category = "testing",
    .init = init
);
// clang-format on
