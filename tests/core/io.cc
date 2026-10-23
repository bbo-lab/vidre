#include <iostream>

#include "../catch2_helper.hh"

using namespace vidre;

#include <vidre/io/endpoint/sink.hh>
#include <vidre/io/packet/base.hh>

class io_capture : public io::sink {
public:
    io_capture() = default;
    virtual ~io_capture() = default;

    cap getcap() const noexcept override {
        using cap = io::endpoint::cap;
        return {.sink = cap::packet | cap::chunk | cap::event | cap::push};
    }

    std::vector<uint8_t> data;

    io::offset_type write(const memory::view &v) override {
        auto first = v.data();
        data.insert(data.end(), first, first + v.size());
        return v.size();
    }

    status put(shared<io::packet> pkt) override {
        if (!pkt)
            throw exception::logic() << "nullptr packet passed";

        auto result = pkt->emit(*this);

        if (!result)
            return status::bad;
        return status::ok;
    }

    memory::view view() const noexcept {
        return {data.size(), data.data()};
    }
};

namespace Catch {
    template <>
    struct StringMaker<io::version_info> {
        static std::string convert(io::version_info const &vi) {
            std::ostringstream oss;
            oss << "{" << vi.major << ", " << vi.minor << "}";
            return oss.str();
        }
    };
    template <>
    struct StringMaker<io::packet::id> {
        static std::string convert(io::packet::id const &id) {
            std::ostringstream oss;
            oss << "{" << id.major << ", " << id.minor << "}";
            return oss.str();
        }
    };
    template <>
    struct StringMaker<io::packet::header> {
        static std::string convert(io::packet::header const &hdr) {
            std::ostringstream oss;
            oss << "{" << "{" << (unsigned)hdr.type.major << ", " << (unsigned)hdr.type.minor << "}, "  //
                << hdr.stream << ", " << hdr.length << "}";
            return oss.str();
        }
    };
}  // namespace Catch

TEST_CASE("Packet basics", "[vidre/io/packet/base]") {
    SECTION("version-info") {
        REQUIRE(io::version < io::version_info{2});
        REQUIRE(io::version <= io::version_info{2});
        REQUIRE(io::version <= io::version);
        REQUIRE(io::version == io::version);
        REQUIRE(io::version >= io::version);
        REQUIRE(io::version >= io::version_info{0, 9});
        REQUIRE(io::version > io::version_info{0, 9});
    }

    SECTION("proto") {
        static constexpr char proto_mem_1[] = {
            0x4F, 0x42, 0x42, 0x65, 0x52, 0x64, 0x69, 0x56,  // magic
            0x00, 0x00, 0x00, 0x01,                          // major
            0x00, 0x00, 0x00, 0x00                           // minor
        };

        CHECK_NOTHROW(io::proto::make(proto_mem_1));

        static constexpr char proto_mem_2[] = {
            0x56, 0x69, 0x64, 0x52, 0x65, 0x42, 0x42, 0x4F,  // magic
            0x01, 0x00, 0x00, 0x00,                          // major
            0x00, 0x00, 0x00, 0x00                           // minor
        };

        CHECK_NOTHROW(io::proto::make(proto_mem_2));
    }

    SECTION("packet-base") {
        PREPARE(shared<io::packet> pkt1 = io::packet::make(io::packet::major::control, 0x89, 253));
        REQUIRE(pkt1->getmajor() == io::packet::major::control);
        REQUIRE(pkt1->getid().minor == 0x89);
        REQUIRE(pkt1->size() == sizeof(io::packet::header));

        io_capture capture;
        PREPARE(auto result = pkt1->emit(capture));
        REQUIRE(result == true);
        REQUIRE(result.count == pkt1->size());

        PREPARE(auto pkt2 = io::packet::decode(capture.view()));
        REQUIRE(pkt2->getheader() == pkt1->getheader());
    }

    SECTION("packet-queue") {
        auto pkt1 = io::packet::make(io::packet::major::control, 1);
        auto pkt2 = io::packet::make(io::packet::major::control, 2);
        auto pkt3 = io::packet::make(io::packet::major::control, 3);

        SECTION("fifo") {
            io::packet::queue q;

            REQUIRE(q.empty());
            REQUIRE(q.size() == 0);
            REQUIRE(q.front() == nullptr);
            REQUIRE(q.pop() == nullptr);

            REQUIRE(q.push(pkt1));
            REQUIRE(q.push(pkt2));
            REQUIRE(q.push(pkt3));

            REQUIRE(q.size() == 3);
            REQUIRE(q.front() == pkt1);

            REQUIRE(q.pop() == pkt1);
            REQUIRE(q.pop() == pkt2);
            REQUIRE(q.pop() == pkt3);

            REQUIRE(q.empty());
            REQUIRE(q.pop() == nullptr);
        }

        SECTION("depth-one") {
            io::packet::queue q{1};

            REQUIRE(q.push(pkt1));
            REQUIRE_FALSE(q.push(pkt2));

            REQUIRE(q.size() == 1);
            REQUIRE(q.front() == pkt2);
            REQUIRE(q.pop() == pkt2);
            REQUIRE(q.empty());
        }

        SECTION("depth-two") {
            io::packet::queue q{2};

            REQUIRE(q.push(pkt1));
            REQUIRE(q.push(pkt2));
            REQUIRE_FALSE(q.push(pkt3));

            REQUIRE(q.size() == 2);
            REQUIRE(q.pop() == pkt2);
            REQUIRE(q.pop() == pkt3);
            REQUIRE(q.empty());
        }

        SECTION("null-packet") {
            io::packet::queue q;

            REQUIRE_FALSE(q.push(nullptr));
            REQUIRE(q.empty());
        }

        SECTION("dynamic-depth") {
            io::packet::queue q{0};

            REQUIRE(q.depth() == io::packet::queue::dynamic);
            REQUIRE(q.push(pkt1));
            REQUIRE(q.push(pkt2));
            REQUIRE(q.push(pkt3));

            REQUIRE(q.size() == 3);
        }
    }
}

#include <vidre/io/packet/value.hh>
#include <vidre/io/packet/sequence.hh>
#include <vidre/io/packet/object.hh>
TEST_CASE("Packet objects", "[vidre/io/packet/object]") {
    using packet = io::packet;
    using value = packet::value;
    using minor = value::minor;

    // Test the complete path through capture::put() and decode().
    auto roundtrip = [](shared<packet> pkt) {
        io_capture capture;
        pkt->finalize();

        REQUIRE(capture.put(pkt) == status::ok);
        REQUIRE(capture.data.size() == pkt->size());

        auto decoded = packet::decode(capture.view());
        REQUIRE(decoded != nullptr);
        REQUIRE(decoded->getheader() == pkt->getheader());
        return decoded;
    };

    SECTION("value-variant") {
        value nil;
        REQUIRE(nil.getid().minor == static_cast<uint8_t>(minor::nil));
        REQUIRE_NOTHROW(nil.get<std::monostate>());
        REQUIRE_THROWS_AS(nil.get<bool>(), std::bad_variant_access);

        value boolean{true};
        REQUIRE(boolean.getid().minor == static_cast<uint8_t>(minor::boolean));
        REQUIRE(boolean.get<bool>());
        REQUIRE_THROWS_AS(boolean.get<int64_t>(), std::bad_variant_access);

        value integer{int32_t{-123}};
        REQUIRE(integer.getid().minor == static_cast<uint8_t>(minor::integer));
        REQUIRE(integer.get<int64_t>() == -123);

        value natural{uint32_t{456}};
        REQUIRE(natural.getid().minor == static_cast<uint8_t>(minor::natural));
        REQUIRE(natural.get<uint64_t>() == 456);

        value number{3.25f};
        REQUIRE(number.getid().minor == static_cast<uint8_t>(minor::number));
        REQUIRE(number.get<double>() == 3.25);

        value string{"hello"};
        REQUIRE(string.getid().minor == static_cast<uint8_t>(minor::string));
        REQUIRE(string.get<std::string>() == "hello");
    }

    SECTION("value-roundtrip") {
        auto check = [&]<typename T>(T input, uint16_t stream = 0) {
            auto original = packet::make<value>(input, stream);
            auto decoded = roundtrip(original);
            auto result = std::dynamic_pointer_cast<value>(decoded);

            REQUIRE(result != nullptr);
            // REQUIRE(result->get<T>() == original->get<T>()); FIXME
            REQUIRE(result->getheader() == original->getheader());
        };

        auto nil = roundtrip(packet::make<value>());
        auto nil_value = std::dynamic_pointer_cast<value>(nil);
        REQUIRE(nil_value != nullptr);
        REQUIRE_NOTHROW(nil_value->get<std::monostate>());

        check(bool{false});
        check(bool{true});
        check(int64_t{-123456789});
        check(uint64_t{123456789});
        check(double{3.141592653589793});
        check(std::string{});
        check(std::string{"hello VidRe"}, 253);
        check(std::string{"a\0b", 3});
    }

    SECTION("value-binary") {
        static constexpr uint8_t bytes[] = {0x00, 0x01, 0x80, 0xff};

        auto original = packet::make<value>(
            memory::view{sizeof(bytes), bytes});

        auto decoded = roundtrip(original);
        auto result = std::dynamic_pointer_cast<value>(decoded);

        REQUIRE(result != nullptr);

        auto binary = result->get<memory::view>();
        REQUIRE(binary.size() == sizeof(bytes));
        REQUIRE(std::equal(binary.begin(), binary.end(), bytes));
    }

    SECTION("sequence") {
        auto original = packet::make<packet::sequence>();
        original->add(packet::make<value>(int64_t{-42}));
        original->add(packet::make<value>(std::string{"test"}));
        original->add(packet::make<value>(true));

        auto decoded = roundtrip(original);
        auto result = std::dynamic_pointer_cast<packet::sequence>(decoded);

        REQUIRE(result != nullptr);
        REQUIRE(result->count() == 3);

        auto a = std::dynamic_pointer_cast<value>(result->packets()[0]);
        auto b = std::dynamic_pointer_cast<value>(result->packets()[1]);
        auto c = std::dynamic_pointer_cast<value>(result->packets()[2]);

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);

        REQUIRE(a->get<int64_t>() == -42);
        REQUIRE(b->get<std::string>() == "test");
        REQUIRE(c->get<bool>());
    }

    SECTION("object") {
        auto original = packet::make<packet::object>(
            value{std::string{"exposure"}},
            value{double{12.5}});

        auto decoded = roundtrip(original);
        auto result = std::dynamic_pointer_cast<packet::object>(decoded);

        REQUIRE(result != nullptr);

        auto key = std::dynamic_pointer_cast<value>(result->key());
        auto val = std::dynamic_pointer_cast<value>(result->value());

        REQUIRE(key != nullptr);
        REQUIRE(val != nullptr);

        REQUIRE(key->get<std::string>() == "exposure");
        REQUIRE(val->get<double>() == 12.5);
    }
}

#if 0

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
