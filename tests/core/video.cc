#include <catch2/catch_test_macros.hpp>

#include <vidre/video/format/pixel.hh>
using namespace vidre;
TEST_CASE("format/pixel.hh", "[vidre/video]") {
    using namespace vidre::video::format;
    REQUIRE(fourcc::rgb332 == pixel(fourcc::rgb332));
    REQUIRE(pixel(fourcc::rgb332).name() == "rgb332");
}
