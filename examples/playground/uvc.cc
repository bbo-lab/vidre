#include <vidre/device/camera.hh>
#include <vidre/log.hh>
#include <vidre/program.hh>
#include <vidre/storage/io.hh>

#include <iostream>
#include <string>
#include <filesystem>

#if VIDRE_HOST_LINUX
#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#elif VIDRE_HOST_WINDOWS
// FIXME
#elif VIDRE_HOST_MACOS
// FIXME
#endif

using namespace vidre;

/*
Notes:
see "UVC Payload Header Metadata"
-> packet timing

/dev/video0 (camera) + /dev/video1 (UVC metadata)


use https://libuvc.github.io/libuvc/
*/

class uvc : public virtual device::camera {
public:
    struct enumerator {
        device::list<device::camera> find() noexcept;
    };

public:
    uvc() = default;
    virtual ~uvc() {
        // FIXME release camera
    }

protected:
#if VIDRE_HOST_LINUX
    std::filesystem::path dev_stream_{};  ///< /dev/video? device for video stream
    std::filesystem::path dev_meta_{};    ///< /dev/video? device for UVC metadata stream
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

static bool is_uvc_camera(log::trace &tr, const std::filesystem::path &devpath, v4l2_capability &cap) noexcept {
    // check if can be opened
    storage::fileio fd = storage::fileio(devpath, storage::fileio::readwrite);
    if (!fd) {
        tr << std::format("-> failed to open device: {}\n", std::strerror(errno));
        close(fd);
        return false;
    }

    // check capabilities
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        tr << std::format("-> failed to query device capabilities: {}\n", std::strerror(errno));
        close(fd);
        return false;
    }

    // found a UVC camera
    tr << std::format("         driver: {}\n", (const char *)cap.driver);
    tr << std::format("           card: {}\n", (const char *)cap.card);
    tr << std::format("       bus_info: {}\n", (const char *)cap.bus_info);
    tr << std::format("        version: {}.{}.{}\n", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
    tr << std::format("   capabilities: 0x{:08X}\n", cap.capabilities);
    tr << std::format("         device: 0x{:08X}\n", cap.device_caps);

    close(fd);

    return true;
}

device::list<device::camera> uvc::enumerator::find() noexcept {
    log::trace tr;
    device::list<device::camera> cameras;  // what is returned

    // try to find UVC cameras on the system and return a list of camera instances
    tr << "Searching for UVC cameras in /dev/video* ...\n";

    bool have_uvch = true;  // flag, if we parse for UVC metadata devices
    int lastcam = -1;       // last camera number found, used to find UVC metadata devices

    for (int dev = 0; dev < 64; ++dev) {
        std::filesystem::path devpath = std::format("/dev/video{}", dev);

        // check if device exists (and get rdev info)
        struct stat st;
        if (stat(devpath.c_str(), &st) < 0)
            continue;

        std::string rdev = std::format("{}:{}", major(st.st_rdev), minor(st.st_rdev));

        tr << std::format("    {} ({})\n", devpath.string(), rdev);

        // get media information if available
        std::filesystem::path sysdir = std::format("/sys/dev/char/{}/device", rdev);
        for (const auto &entry : std::filesystem::directory_iterator(sysdir)) {
            auto filename = entry.path().filename().string();

            if (filename.find("media") == 0) {
                tr << std::format("         {}:\n", filename);

                storage::fileio fd = storage::fileio(std::format("/dev/{}", filename), storage::fileio::readonly);
                if (fd.valid()) {
                    struct media_device_info mdi;
                    if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdi) == 0) {
                        tr << std::format("             driver:   {}\n", (const char *)mdi.driver);
                        tr << std::format("             model:    {}\n", (const char *)mdi.model);
                        tr << std::format("             serial:   {}\n", (const char *)mdi.serial);
                        tr << std::format("             bus_info: {}\n", (const char *)mdi.bus_info);
                        tr << std::format("             version:  {}.{}.{}\n", (mdi.media_version >> 16) & 0xFF,
                                          (mdi.media_version >> 8) & 0xFF, mdi.media_version & 0xFF);

                        // FIXME use information
                    }
                }

                break;
            }
        }

        // get capabilities and check if this is a UVC camera
        storage::fileio fd = storage::fileio(devpath, storage::fileio::readwrite);
        if (!fd) {
            tr << std::format("-> failed to open device: {}\n", std::strerror(errno));
            continue;
        }

        v4l2_capability cap;
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
            tr << std::format("-> failed to query device capabilities: {}\n", std::strerror(errno));
            continue;
        }


        // FIXME
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

    // int fd = open("/dev/media1", O_RDWR);
    // if (fd < 0) {
    //     perror("Failed to open /dev/media0");
    //     return 1;
    // }

    // struct media_device_info mdi;
    // if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdi) == 0) {
    //     printf("Media Driver Info:\n");
    //     printf("Driver name    : %s\n", mdi.driver);
    //     printf("Model          : %s\n", mdi.model);
    //     printf("Serial         : %s\n", mdi.serial);
    //     printf("Bus info       : %s\n", mdi.bus_info);
    //     printf("Media version  : %u.%u.%u\n", (mdi.media_version >> 16) & 0xFF, (mdi.media_version >> 8) & 0xFF,
    //            mdi.media_version & 0xFF);
    //     printf("Hardware revision: 0x%x\n", mdi.hw_revision);
    // }
    // else {
    //     perror("MEDIA_IOC_DEVICE_INFO failed");
    // }

    // close(fd);

    return 0;
}
