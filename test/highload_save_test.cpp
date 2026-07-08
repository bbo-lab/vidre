#include <core/runtime/runtime.hpp>
#include <camera/dummy/dummy_camera.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

namespace po = boost::program_options;

namespace {
struct Resolution {
    unsigned int width;
    unsigned int height;
};

std::vector<Resolution> parseResolutions(const std::string& text) {
    std::vector<Resolution> resolutions;
    std::stringstream stream(text);
    std::string token;

    while (std::getline(stream, token, ',')) {
        const auto separator = token.find('x');
        if (separator == std::string::npos) {
            continue;
        }

        const auto widthText = token.substr(0, separator);
        const auto heightText = token.substr(separator + 1);

        try {
            const auto width = static_cast<unsigned int>(std::stoul(widthText));
            const auto height = static_cast<unsigned int>(std::stoul(heightText));
            resolutions.push_back({width, height});
        } catch (...) {
            continue;
        }
    }

    return resolutions;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string outputFile = "highload.ccv";
    std::string headerFormat = "ccv";
    std::string resolutionsText = "640x480,1280x720,1920x1080";
    unsigned int width = 1280;
    unsigned int height = 720;
    unsigned int buffers = 500;
    double frameRate = 0.0;
    int durationSeconds = 10;

    po::options_description options("Allowed options");
    options.add_options()
        ("help,h", "Print this help message")
        ("output,o", po::value<std::string>(&outputFile)->default_value(outputFile), "Output CCV file")
        ("header", po::value<std::string>(&headerFormat)->default_value(headerFormat), "Header format")
        ("resolutions,r", po::value<std::string>(&resolutionsText)->default_value(resolutionsText), "Comma-separated widthxheight list, for example 640x480,1280x720")
        ("width", po::value<unsigned int>(&width)->default_value(width), "Frame width")
        ("height", po::value<unsigned int>(&height)->default_value(height), "Frame height")
        ("buffers", po::value<unsigned int>(&buffers)->default_value(buffers), "Shared frame buffer count")
        ("frame-rate", po::value<double>(&frameRate)->default_value(frameRate), "Dummy camera frame rate")
        ("duration", po::value<int>(&durationSeconds)->default_value(durationSeconds), "Run duration in seconds");

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, options), variables);
    po::notify(variables);

    if (variables.count("help") != 0) {
        std::cout << options << '\n';
        return 0;
    }

    auto resolutions = parseResolutions(resolutionsText);
    if (resolutions.empty()) {
        resolutions.push_back({width, height});
    }

    for (const auto& resolution : resolutions) {
        auto camera = std::make_unique<vidre::DummyCamera>();
        camera->SetMaxNumBuffer(buffers);
        camera->SetFrameWidth(resolution.width);
        camera->SetFrameHeight(resolution.height);
        camera->SetFrameRate(frameRate);
        camera->SetFrameOffsetX(0);
        camera->SetFrameOffsetY(0);
        camera->Init("dummy-highload");

        const std::filesystem::path requestedOutputPath(outputFile);
        const auto resolutionPrefix = std::to_string(resolution.width) + "x" + std::to_string(resolution.height) + "_";
        const std::filesystem::path runOutputPath = requestedOutputPath.parent_path() /
            (resolutionPrefix + requestedOutputPath.filename().string());
        vidre::RuntimeInstance runtime;
        runtime.setCamera(std::move(camera));
        runtime.createStorage(runOutputPath.string(), headerFormat, 8, 0);

        if (!runtime.checkCamera()) {
            std::cerr << "Dummy camera was not created for " << resolution.width << "x" << resolution.height << '\n';
            return 1;
        }

        std::cout << "Running " << resolution.width << "x" << resolution.height
                  << " for " << durationSeconds << " s" << '\n';

        const auto startTime = std::chrono::steady_clock::now();
        runtime.run();
        std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));
        runtime.stop();
        const auto endTime = std::chrono::steady_clock::now();

        const auto elapsedSeconds = std::chrono::duration<double>(endTime - startTime).count();
        if (!std::filesystem::exists(runOutputPath)) {
            std::cout << "Finished " << resolution.width << "x" << resolution.height
                      << ", but no output file was found at " << runOutputPath.string() << '\n';
            continue;
        }

        const auto fileBytes = std::filesystem::file_size(runOutputPath);
        const double mebibytes = static_cast<double>(fileBytes) / (1024.0 * 1024.0);
        const double speed = mebibytes / elapsedSeconds;

        std::cout << "Saved " << runOutputPath.filename().string()
                  << " (" << fileBytes << " bytes, " << mebibytes << " MiB)"
                  << ", average write speed: " << speed << " MiB/s" << '\n';
    }

    std::cout << "Finished multi-resolution high-load storage run" << '\n';
    return 0;
}