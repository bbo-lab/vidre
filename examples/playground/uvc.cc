#include <vidre/device/camera.hh>
#include <vidre/log.hh>
#include <vidre/program.hh>
#include <vidre/storage/io.hh>

#include <iostream>
#include <string>
#include <filesystem>

#if VIDRE_HOST_LINUX
#include <linux/videodev2.h>
#elif VIDRE_HOST_WINDOWS
#elif VIDRE_HOST_MACOS
#endif

using namespace vidre;

class uvc : public virtual device::camera {
public:
    struct enumerator {
        device::list<device::camera> find() noexcept;
    };

public:
    uvc() = default;
    uvc(std::filesystem::path fspath) : device::camera(device::state::available), fspath_{std::move(fspath)} {}
    uvc(std::filesystem::path fspath, v4l2_capability &capabilities)
        : device::camera(device::state::available), fspath_{std::move(fspath)}, capabilities_{capabilities} {}
    virtual ~uvc() {
        // FIXME release camera
    }

protected:
#if VIDRE_HOST_LINUX
    std::filesystem::path fspath_{};
    v4l2_capability capabilities_{};
#elif VIDRE_HOST_WINDOWS
#elif VIDRE_HOST_MACOS
#endif

protected:
#if VIDRE_HOST_LINUX
#elif VIDRE_HOST_WINDOWS
#elif VIDRE_HOST_MACOS
#endif

public:
    const char *tag() const noexcept override {
        return "camera/uvc";
    }

    settings::json info() const noexcept override;
    void setup(settings::json) override;
    std::string id() const noexcept override;
    std::string name() const noexcept override;
    bool name(const std::string &username) override;
};

#if VIDRE_HOST_LINUX

device::list<device::camera> uvc::enumerator::find() noexcept {
    log::trace tr;

    // try to find UVC cameras on the system and return a list of camera instances
    device::list<device::camera> cameras;

    // 1. try /dev/v4l/by-id/usb-*-video-index0
    if (std::filesystem::exists("/dev/v4l/by-id/")) {
        tr << "Searching for UVC cameras in /dev/v4l/by-id/ ...\n";
        for (const auto &entry : std::filesystem::directory_iterator("/dev/v4l/by-id/")) {
            tr << std::format("- checking {}\n", entry.path().string());

            if (entry.is_symlink() && entry.path().filename().string().find("usb-") == 0 &&
                entry.path().filename().string().find("-video-index0") != std::string::npos)
            {
                // check if can be opened
                storage::fileio fd = storage::fileio(entry.path(), storage::fileio::readwrite);
                if (!fd) {
                    tr << std::format("-> failed to open device: {}\n", std::strerror(errno));
                    close(fd);
                    continue;
                }

                // check capabilities
                v4l2_capability cap;
                if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
                    tr << std::format("-> failed to query device capabilities: {}\n", std::strerror(errno));
                    close(fd);
                    continue;
                }

                // found a UVC camera
                tr << std::format("-> found UVC camera: {} ({})\n", (const char *)(cap.card), (const char *)(cap.bus_info));
                cameras.emplace_back(std::make_shared<uvc>(entry.path(), cap));

                close(fd);
            }
        }
    }

    if (not cameras.empty())
        return cameras;

    // 2. try /dev/video*
    for (const auto &entry : std::filesystem::directory_iterator("/dev/")) {
        if (entry.is_character_file() && entry.path().filename().string().find("video") == 0) {
            // FIXME check if it's actually a UVC camera and not some other video device
            // FIXME create a uvc camera instance for this device and add it to the list
        }
    }

    return cameras;
}

settings::json uvc::info() const noexcept {
    // FIXME return camera info as JSON
    return {};
}

void uvc::setup(settings::json) {
    // FIXME setup camera
}
std::string uvc::id() const noexcept {
    // FIXME return unique camera ID
    return "uvc-camera-1";
}
std::string uvc::name() const noexcept {
    // FIXME return camera name
    return "UVC Camera 1";
}
bool uvc::name(const std::string &username) {
    // FIXME set camera name
    return false;
}

#endif

// ============================================================================

void version(program::argument::parser &, variant) {
    std::cout << VIDRE_VERSION_S << std::endl;
    std::exit(0);
}

void help(program::argument::parser &p, variant) {
    std::cout << p.help();
    std::exit(0);
}

void log_level(program::argument::parser &, variant value) {
    log::setup(value);
    if (std::string(value) == "help")
        std::exit(0);
}

void list(program::argument::parser &, variant) {
    // list cameras
    uvc::enumerator enumerator;
    auto cameras = enumerator.find();
    for (const auto &camera : cameras) {
        std::cout << camera->name() << " (" << camera->id() << ")" << std::endl;
    }
    std::exit(0);
}

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    try {
        //! [program-options]
        program::options args(  //
            "UVC playground",   //
            "Copyright (C) 2024-2026 MPI f. Neurobiol. of Behavior—ceasar, Bonn, Germany", "UVC driver development playground.",
            {
                {"help", 'h', "Show this usage information.", help},
                {"version", 'V', "Display program version", version},
                {"verbose", 'v', "Increase verbosity"},
                {"list", "List available cameras", list},
                {"log", "Log level (use 'help' for further info).", program::argument::optional, "info", log_level},
                program::argument::parameter{"output", "Output file (default: stdout)", "/dev/stdout"},
            });
        //! [program-options]

        program::argument::parser parser(args);
        parser.parse(argc, argv);

        // FIXME run camera and write output to file
    } catch (exception &e) {
        std::cerr << error::diagnostic(e).dump() << std::endl;
    }

    return 0;
}
