#include <catch2/catch_test_macros.hpp>
#include <vidre/program.hh>

using namespace vidre;

#include <vidre/util/macros.hh>
#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

TEST_CASE("detail::tinymap", "[program.hh]") {
    PREPARE(program::detail::tinymap map;);
    REQUIRE(map.empty());

    PREPARE(map.set("home", "WLAN"));
    REQUIRE(map.size() == 1);

    // FIXME etc.
}

void argcb_log_debug(program::argument::parser &, const std::string &key, variant value) {
    // nothing's done here, just an stub
}

TEST_CASE("option", "[application.hh]") {
    // This test here only test "compilability".
    {
        std::initializer_list<program::argument> args{{"help", "Display help."}, {"help", 'h', "Display help."}};
    }
    {
        std::initializer_list<program::argument> args{{"infile", "Input file.", program::argument::positional}};
        // std::initializer_list<program::argument> args_alt{program::argument::positional{"infile", "Input file."}};
    }
    {
        std::initializer_list<program::argument> args{
            {"help", 'h', "Display help."},                              //
            {"log-debug", "Set debug logging level.", argcb_log_debug},  //
        };
        REQUIRE(args.size() == 2);
    }
    {
        program::options decl(" - brief",      //
                              "Description.",  //
                              {
                                  {"help", "Display help."},       //
                                  {"help", 'h', "Display help."},  //
                              });
    }
}
