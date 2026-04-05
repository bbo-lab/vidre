#include <catch2/catch_test_macros.hpp>
#include <vidre/plugin/dynamic.hh>

using namespace vidre;

#if defined(VIDRE_USING_DLFCN)
#if defined(_GNU_SOURCE)
#include <gnu/lib-names.h>
#else
#define LIBM_SO "libm.so"
#endif
#elif defined(VIDRE_USING_LIBLOADERAPI)
#error FIXME
#endif

using unaryfn = double (*)(double);

TEST_CASE("Plugins", "[dynamic:library]") {
#if defined(VIDRE_USING_DLFCN)
    plugin::dynamic::library dl(LIBM_SO);
    REQUIRE(dl.valid());
    REQUIRE(dl.get("sin"));
    auto libmsin = dl.get<unaryfn>("sin");
    REQUIRE(libmsin(0.0) == 0.0);
#elif defined(VIDRE_USING_LIBLOADERAPI)
#error FIXME
#endif
}

TEST_CASE("Plugins", "[dynamic]") {
    plugin::dynamic null;
    null.load(Vidre_BINARY_DIR "/lib/test/null");
    REQUIRE(null.valid());
    REQUIRE(std::string(null.info().name) == "TestNull");
}