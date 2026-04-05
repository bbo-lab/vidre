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

#include <vidre/util/version.hh>

namespace Catch {
    template <>
    struct StringMaker<vidre::version> {
        static std::string convert(const vidre::version &v) {
            return std::format("{}.{}.{}", v.major() != vidre::version::undefined ? std::to_string(v.major()) : "?",
                               v.minor() != vidre::version::undefined ? std::to_string(v.minor()) : "?",
                               v.patch() != vidre::version::undefined ? std::to_string(v.patch()) : "?");
        }
    };
}  // namespace Catch

TEST_CASE("version.hh", "[vidre/util]") {
    REQUIRE(version::vidre == version{VIDRE_VERSION_MAJOR, VIDRE_VERSION_MINOR, VIDRE_VERSION_PATCH});
    REQUIRE(version::vidre == version{std::to_string(VIDRE_VERSION_MAJOR) + "." + std::to_string(VIDRE_VERSION_MINOR) + "." +
                                      std::to_string(VIDRE_VERSION_PATCH)});

    REQUIRE(version{1} == version{1, 0});
    REQUIRE(version{1, 0} <= version{1, 0, 0});
    REQUIRE(version{1, 0} >= version{1, 0, 0});
    REQUIRE(version{1, 0} < version{1, 1});
    REQUIRE(version{1, 0} < version{2});

    REQUIRE(not (version{} != version{}));
    REQUIRE(not (version{} == version{}));
}
