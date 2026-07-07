#include <core/runtime/runtime.hpp>
#include <camera/dummy/dummy_camera.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    std::string outputFile = "highload.ccv";
    std::string headerFormat = "ccv";
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

    auto camera = std::make_unique<vidre::DummyCamera>();
    camera->SetMaxNumBuffer(buffers);
    camera->SetFrameWidth(width);
    camera->SetFrameHeight(height);
    camera->SetFrameRate(frameRate);
    camera->SetFrameOffsetX(0);
    camera->SetFrameOffsetY(0);
    camera->Init("dummy-highload");

    vidre::RuntimeInstance runtime;
    runtime.setCamera(std::move(camera));
    runtime.createStorage(outputFile, headerFormat, 8, 0);

    if (!runtime.checkCamera()) {
        std::cerr << "Dummy camera was not created\n";
        return 1;
    }

    runtime.run();
    std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));
    runtime.stop();

    std::cout << "Finished high-load storage run\n";
    return 0;
}