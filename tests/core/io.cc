#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <iostream>

#include <vidre/util/macros.hh>
#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

template <typename T, std::size_t N>
struct array_wrapper : std::array<T, N> {
    template <typename... ArgTs>
    constexpr array_wrapper(ArgTs &&...args) : std::array<T, N>{std::forward<ArgTs>(args)...} {}
};

#include <vidre/io/types.hh>
using namespace vidre;
TEST_CASE("types.hh", "[vidre/io]") {
    std::cout << (sizeof(io::result)) << std::endl;
}


#if 0
#include <vidre/storage/bit.hh>
using namespace vidre;
TEST_CASE("bit.hh", "[vidre/storage]") {
    {
        const uint16_t input[2] = {0b1100110011, 0b1001001001};
        uint8_t output[4] = {};

        // 0b1100110011 = 819, 0b1001001001 = 585
        // 0b00110011 = 51, 0b00100111 = 39, 0b1001 = 9

        storage::view src(4, reinterpret_cast<const uint8_t *>(input));
        storage::view snk(4, output);

        auto retval = storage::pack<16, 10>(src, snk);
        REQUIRE(retval == 3);

        uint16_t reverse[2] = {};
        storage::view revsrc(retval, output);
        storage::view revsnk(4, reverse);

        retval = storage::pack<10, 16>(revsrc, revsnk);
        REQUIRE(retval == 4);
        REQUIRE(memcmp(input, reverse, 4) == 0);
    }

    {
#define NSAMPLES 16384
        uint16_t source[NSAMPLES];
        for (auto &item : source)
            item = GENERATE(take(1, random(0, 0b1111111111)));

        uint8_t packed[storage::pack_size<16, 10>(NSAMPLES)];
        uint8_t unpacked[NSAMPLES];

        REQUIRE(storage::pack_size<16, 10>(NSAMPLES) == storage::pack<16, 10>(source, packed));
        REQUIRE(NSAMPLES == storage::pack<10, 16>(packed, unpacked));

        REQUIRE(memcmp(source, unpacked, NSAMPLES) == 0);
    }

    {
#define NSAMPLES 16384
        uint16_t source[NSAMPLES];
        for (auto &item : source)
            item = GENERATE(take(1, random(0, 0b111111111111)));

        uint8_t packed[storage::pack_size<16, 12>(NSAMPLES)];
        uint8_t unpacked[NSAMPLES];

        REQUIRE(storage::pack_size<16, 12>(NSAMPLES) == storage::pack<16, 12>(source, packed));
        REQUIRE(NSAMPLES == storage::pack<12, 16>(packed, unpacked));

        REQUIRE(memcmp(source, unpacked, NSAMPLES) == 0);
    }
}
#endif
