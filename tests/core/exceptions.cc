#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <vidre/exception.hh>
#include <iostream>
#include <random>

using namespace vidre;

TEST_CASE("detail::exception", "[exceptions.hh]") {
    using store = detail::exception::store;

    {
        // try single allocation and deallocation
        void *ptr;
        ptr = store::allocate(10);
        store::deallocate(ptr);
        REQUIRE(store::free() == store::max_available);
    }

    {
        // try two allocations, deallocation in order
        void *a = store::allocate(10);
        void *b = store::allocate(243);
        store::deallocate(b);
        store::deallocate(a);
        REQUIRE(store::free() == store::max_available);
    }

    {
        // try two allocations, deallocation reverse order
        void *a = store::allocate(10);
        void *b = store::allocate(243);
        store::deallocate(a);
        store::deallocate(b);
        REQUIRE(store::free() == store::max_available);
    }

    {
        // random allocations
        std::vector<void *> allocs;
        while (true) {
            size_t len = GENERATE(take(1, random(1, 1023)));

            if (store::free() < len)
                break;

            auto ptr = store::allocate(len);
            if (ptr == nullptr) {
                REQUIRE(ptr != nullptr);
            }
            allocs.push_back(ptr);
        }

        REQUIRE(store::free() == store::free());  // only for reporting...

        // random deallocation
        while (not allocs.empty()) {
            size_t n = 0;
            if (allocs.size() > 1) {
                n = GENERATE(take(1, random(0, int(VIDRE_EXCEPTION_STORE))));
                n %= (allocs.size() - 1);
            }

            store::deallocate(allocs[n]);
            allocs.erase(allocs.begin() + n);
        }

        REQUIRE(store::free() == store::max_available);
    }

    {
        using exception_string = std::basic_string<char, std::char_traits<char>, detail::exception::allocator<char>>;

        {
            exception_string str = "Hello error! This is a long message that does not fit into std::string local storage...";
            REQUIRE(store::free() != store::max_available);
        }

        REQUIRE(store::free() == store::max_available);
    }
}

TEST_CASE("error::info", "[exceptions.hh]") {
    using store = detail::exception::store;

    {               
        {
            error::info info [[maybe_unused]]{};

            REQUIRE(std::string(info.tag()) == "<>");
            REQUIRE(store::free() == store::max_available);

            info.attach(error::info());
            REQUIRE(store::free() != store::max_available);
        }

        REQUIRE(store::free() == store::max_available);
    }

    {
        {
            error::message info [[maybe_unused]] = "some explanation for an error...";

            REQUIRE(std::string(info.tag()) == "message");

            REQUIRE(store::free() != store::max_available);
        }

        REQUIRE(store::free() == store::max_available);
    }

    {
        {
            exception e [[maybe_unused]]{};

            e << error::info() << error::location();
            REQUIRE(store::free() != store::max_available);

            error::diagnostic diag(e);
            error::string emsg = diag.dump();
            REQUIRE(store::free() != store::max_available);

            std::cerr << emsg << std::endl;
        }

        REQUIRE(store::free() == store::max_available);
    }

    {
        try {
            throw exception::runtime()  //
                << error::location()    //
                << error::message("this is an error");
        } catch (exception &e) {
            std::cerr << error::diagnostic(e).dump() << std::endl;
        }

        REQUIRE(store::free() == store::max_available);
    }
}
