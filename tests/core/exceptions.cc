#include <catch2/catch_test_macros.hpp>
#include <vidre/exception.hh>
#include <iostream>

TEST_CASE("Exceptions", "[exception]") {
    try {
        throw vidre::exception() << vidre::throw_message("additional information...");
    }
    catch(const vidre::exception &e) {
        REQUIRE(*boost::get_error_info<boost::throw_line>(e) == 7);
        std::cerr << diagnostic_information(e);
    }
}
