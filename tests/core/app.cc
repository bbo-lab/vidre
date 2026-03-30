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

void argcb_log_debug(arg::parser &, const std::string &key, arg::variant value) {
    // nothing's done here, just an stub
}

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
            {"help", 'h', "Display help."},                              //
            {"log-debug", "Set debug logging level.", argcb_log_debug},  //
        };
        REQUIRE(args.size() == 2);
    }
    {
        arg::option::set decl(" - brief",      //
                              "Description.",  //
                              {
                                  {"help", "Display help."},       //
                                  {"help", 'h', "Display help."},  //
                              });
    }
}
