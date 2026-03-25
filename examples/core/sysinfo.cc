#include <vidre/log.hh>
#include <vidre/util/sysinfo.hh>

using namespace vidre;

struct show_memory {
    show_memory(std::size_t bytes) : n{bytes} {}

    std::size_t n;

    template <typename Stream>
    Stream &operator()(Stream &outs) const noexcept {
        outs << std::format("{:8}", n / (1024*1024)) << " MiB";
        return outs;
    }
};

int main(int, const char **) {
    log::info info{};

    // === memory
    auto mem = sysinfo::memory::get();

    info << "Memory:\n";

    info << "    physical memory size: " << show_memory(mem.total) << log::endl;
    info << "               used size: " << show_memory(mem.used) << log::endl;
    info << "             free memory: " << show_memory(mem.total - mem.used) << log::endl;
    info << "        available memory: " << show_memory(mem.available) << log::endl;

    info << log::endl;
    info << " configuration directory: " << std::string(sysinfo::configdir()) << log::endl;
    info << "     temporary directory: " << std::string(sysinfo::tmpdir()) << log::endl;
    info << "         cache directory: " << std::string(sysinfo::cachedir()) << log::endl;

    return 0;
}

/**@example examples/core/sysinfo.cc
 * @brief System information.
 *
 */
