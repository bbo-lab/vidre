#ifndef CATCH2_HELPER_HH
#define CATCH2_HELPER_HH
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <cstdio>
#include <cctype>
#include <cstddef>
#include <cstdint>

#include <vidre/util/macros.hh>
#include <vidre/memory/base.hh>

#define PREPARE(...)                             \
    UNSCOPED_INFO(VIDRE_STRINGIFY(__VA_ARGS__)); \
    __VA_ARGS__

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

#endif  // CATCH2_HELPER_HH