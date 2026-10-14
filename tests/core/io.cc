#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <iostream>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <vidre/memory/base.hh>
#include "vidre/io/packet/base.hh"
#include "vidre/util/chrono.hh"

void hexdump(vidre::memory::view view) {
    const auto *p = static_cast<const std::uint8_t *>(view.data());

    for (std::size_t offset = 0; offset < view.size(); offset += 16) {
        std::printf("%08zx  ", offset);

        // Hexadecimal representation
        for (std::size_t i = 0; i < 16; ++i) {
            if (offset + i < view.size())
                std::printf("%02x ", p[offset + i]);
            else
                std::printf("   ");

            if (i == 7)
                std::printf(" ");
        }

        std::printf(" |");

        // ASCII representation
        const auto n = (view.size() - offset < 16) ? view.size() - offset : 16;

        for (std::size_t i = 0; i < n; ++i) {
            const auto c = p[offset + i];
            std::printf("%c", std::isprint(static_cast<unsigned char>(c)) ? static_cast<char>(c) : '.');
        }

        std::printf("|\n");
    }

    std::printf("%08zx\n", view.size());
}

#include <vidre/util/macros.hh>
#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

template <typename T, std::size_t N>
struct array_wrapper : std::array<T, N> {
    template <typename... ArgTs>
    constexpr array_wrapper(ArgTs &&...args) : std::array<T, N>{std::forward<ArgTs>(args)...} {}
};

#include <vidre/io/endpoint.hh>
using namespace vidre;

class memory_sink : public io::sink {
public:
    memory_sink() = default;
    virtual ~memory_sink() = default;

    cap getcap() const noexcept override {
        return {};
    }

public:
    std::vector<uint8_t> data;

    io::offset_type write(const memory::view &v) override {
        auto first = v.data();
        data.insert(data.end(), first, first + v.size());
        return v.size();
    }

    memory::view view() const noexcept {
        return {data.size(), data.data()};
    }

    shared<io::packet> parse() const;
};

TEST_CASE("types.hh", "[vidre/io]") {
    // FIXME
}

#include <vidre/io/packet.hh>

shared<io::packet> memory_sink::parse() const {
    auto pkt = io::packet::decode(view());
    pkt->absorb(view());
    return pkt;
}

TEST_CASE("packet.hh", "[vidre/io]") {
    using namespace vidre::io;

#if 0
#define test_io_packet_value(TYPE, VALUE)                                                           \
    {                                                                                               \
        memory_sink sink;                                                                           \
        TYPE value = VALUE;                                                                         \
        packet::value a(value);                                                                     \
        a.emit(sink);                                                                               \
        if (1)                                                                                      \
            hexdump(sink.view());                                                                   \
        if constexpr (std::is_trivially_copyable_v<TYPE>) {                                         \
            REQUIRE(*reinterpret_cast<TYPE *>(sink.data.data() + sizeof(packet::header)) == value); \
        }                                                                                           \
        shared<io::packet> parsed = sink.parse();                                                   \
        REQUIRE(dynamic_cast<packet::value *>(parsed.get()) != nullptr);                            \
        REQUIRE(dynamic_cast<packet::value *>(parsed.get())->get<TYPE>() == value);                 \
    }

    test_io_packet_value(bool, true);
    test_io_packet_value(bool, false);
    test_io_packet_value(int64_t, -123456789);
    test_io_packet_value(uint64_t, 123U);
    test_io_packet_value(double, 0.0);
    test_io_packet_value(double, -1.0);
    test_io_packet_value(double, 1.0);
    test_io_packet_value(double, 3.14159);
    test_io_packet_value(double, std::numeric_limits<double>::infinity());
    test_io_packet_value(double, -std::numeric_limits<double>::infinity());
    // test_io_packet_value(double, std::numeric_limits<double>::quiet_NaN());
    {
        memory_sink sink;
        double value = std::numeric_limits<double>::quiet_NaN();
        packet::value a(value);
        a.emit(sink);
        if (0)
            hexdump(sink.view());
        REQUIRE(std::isnan(*reinterpret_cast<double *>(sink.data.data() + sizeof(packet::header))));
        shared<io::packet> parsed = sink.parse();
        REQUIRE(dynamic_cast<packet::value *>(parsed.get()) != nullptr);
        REQUIRE(std::isnan(dynamic_cast<packet::value *>(parsed.get())->get<double>()));
    }
    test_io_packet_value(std::string, "Hello World!");
    test_io_packet_value(chrono::isodate, chrono::isodate::now());
    test_io_packet_value(chrono::utcstamp, chrono::utcstamp::now());
#endif

    {
        memory_sink sink;

        io::packet::sequence seq;
        seq.add(io::packet::make<io::packet::value>(42));
        seq.add(io::packet::make<io::packet::value>(std::string("hello world")));
        seq.add(io::packet::make<io::packet::value>(uuid::make()));
        seq.add(io::packet::make<io::packet::value>(chrono::utctime::now()));
        seq.add(io::packet::make<io::packet::value>(chrono::isodate::now()));
        seq.add(io::packet::make<io::packet::object>("key","value"));
        seq.emit(sink);

        hexdump(sink.view());

        // std::cout << seq.to_json() << std::endl;

        shared<io::packet> reread = io::packet::decode(sink.view());
        // std::cout << reread->to_json() << std::endl;

        REQUIRE(seq.to_json() == reread->to_json());
    }
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
