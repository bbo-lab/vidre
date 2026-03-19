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

    try {
        throw vidre::system_error(std::make_error_code(std::errc::bad_address));
    }
    catch(const vidre::system_error &e) {
        REQUIRE(*boost::get_error_info<boost::throw_line>(e) == 15);
        REQUIRE(std::make_error_code(std::errc::bad_address).message() == e.what());
        std::cout << diagnostic_information(e);
    }
}
