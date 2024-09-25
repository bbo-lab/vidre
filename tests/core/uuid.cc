#include <catch2/catch_test_macros.hpp>
#include <vidre/util/uuid.hh>

using namespace vidre;

TEST_CASE("UUID", "[type]") {
    REQUIRE(uuid().tostring() == "00000000-0000-0000-0000-000000000000");
    REQUIRE(uuid() == uuid::variant::NCS);
    REQUIRE(uuid() == uuid::type::NIL);
    REQUIRE(uuid::makeRandom() == uuid::variant::DCE);
    REQUIRE(uuid::makeRandom() == uuid::type::Random);
    // ...
}
