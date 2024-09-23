#include <vidre/plugin.hh>

// clang-format off
VIDRE_MODULE(
    .name = "MyModule",
    .version = "0.0",
    .organization = {
        .name = "George"
    },
    .brief = "Example module that does nothing.",
    .category = "example"
);
// clang-format on

extern "C" void init();
extern "C" void depinit();

void init() {
    depinit();
}