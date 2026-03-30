#include <catch2/catch_test_macros.hpp>
#include <vidre/application.hh>

using namespace vidre;

#include <vidre/util/macros.hh>
#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

TEST_CASE("variant", "[application.hh]") {
    {
        PREPARE(arg::variant v{};);
        REQUIRE(v.empty());
        REQUIRE(v.type() == arg::variant::none);
    }
    {
        PREPARE(arg::variant v = 1;);
        REQUIRE(not v.empty());
        REQUIRE(v.type() == arg::variant::integer);
        REQUIRE((int)v == 1);
        REQUIRE_THROWS((const char *)v);
        PREPARE(v = 10;);
        REQUIRE((int)v == 10);
    }
    // FIXME etc.
}

TEST_CASE("map", "[application.hh]") {
    PREPARE(arg::map map;);
    REQUIRE(map.empty());

    PREPARE(map.set("home", "WLAN"));
    REQUIRE(map.size() == 1);

    // FIXME etc.
}

void log_level(parser &, const std::string &key, variant value) {}

TEST_CASE("option", "[application.hh]") {
    // This test here only test "compilability".
    {
        std::initializer_list<arg::option> args{{"help", "Display help."}, {"help", 'h', "Display help."}};
    }
    {
        std::initializer_list<arg::option> args{{"infile", "Input file.", arg::argument::positional}};
        std::initializer_list<arg::option> args_alt{arg::option::positional{"infile", "Input file."}};
    }
    {
        std::initializer_list<arg::option> args{
            {"help", 'h', "Display help."},  //
            {"log-debug", log_level},
        };

        //  *
        //  *      {"<long-option>","<brief>",argument::required,[<value-type>=variant::cstring]}
        //  *      {"<long-option>",<short-option>,"<brief>",argument::required,[<value-type>=variant::cstring]}
        //  *
        //  *      {"<long-option>","<brief>",argument::optional,<default>}
        //  *      {"<long-option>",<short-option>,"<brief>",argument::optional,<default>}
        //  *
        //  *      {"<long-option>","<brief>",argument::positional,[<value-type>=variant::cstring]}
    }
}
