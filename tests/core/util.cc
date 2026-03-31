#include <catch2/catch_test_macros.hpp>

#include <vidre/util/macros.hh>
#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

#include <vidre/util/variant.hh>
using namespace vidre;
TEST_CASE("variant.hh", "[vidre/util]") {
    {
        PREPARE(variant v{};);
        REQUIRE(v.empty());
        REQUIRE(v.type() == variant::none);
    }
    {
        PREPARE(variant v = 1;);
        REQUIRE(not v.empty());
        REQUIRE(v.type() == variant::integer);
        REQUIRE((int)v == 1);
        REQUIRE_THROWS((const char *)v);
        PREPARE(v = 10;);
        REQUIRE((int)v == 10);
    }
    // FIXME etc.
}

#include <vidre/util/uuid.hh>
TEST_CASE("uuid.hh", "[vidre/util]") {
    REQUIRE(uuid().tostring() == "00000000-0000-0000-0000-000000000000");
    REQUIRE(uuid() == uuid::variant::NCS);
    REQUIRE(uuid() == uuid::type::NIL);
    REQUIRE(uuid::makeRandom() == uuid::variant::DCE);
    REQUIRE(uuid::makeRandom() == uuid::type::Random);
    // ...
}
