#include <utils/config_loader/config_loader.hpp>

#include <camera/camera.hpp>
#include <core/runtime/runtime.hpp>
#include <utils/dynamic_loader/dynamic_library_loader.hpp>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace vidre {
    namespace {
        void LogMessage(const LoadSetupOptions& options, const std::string& message) {
            if (options.log) {
                options.log(message);
            } else {
                std::cout << message << std::endl;
            }
        }

        std::string ReadStringByKeys(const YAML::Node& node, const std::vector<std::string>& keys) {
            if (!node || !node.IsMap()) {
                return "";
            }
            for (const auto& key : keys) {
                if (node[key] && node[key].IsScalar()) {
                    return node[key].as<std::string>();
                }
            }
            return "";
        }

        void ApplyPath(const std::string& value, std::string& destination, bool cliOverride,
                       const char* label, const LoadSetupOptions& options) {
            if (!value.empty() && !cliOverride) {
                destination = value;
                LogMessage(options, std::string("Config library path (") + label + "): " + value);
            }
        }

        std::unique_ptr<ICamera> LoadCamera(const std::string& cameraName, const YAML::Node& cameraNode,
                                            LoadSetupOptions& options) {
            vidre::APTINA_LIBRARY_PATH = options.aptinaLibraryPath;
            vidre::PYLON_LIBRARY_PATH = options.pylonLibraryPath;

            if (!cameraNode["vendor"]) {
                LogMessage(options, "ERROR: No vendor specified for camera " + cameraName);
                return nullptr;
            }

            const std::string vendor = cameraNode["vendor"].as<std::string>();
            std::unique_ptr<ICamera> camera;

            if (vendor == "pylon" &&
                (options.pylonBaseLibraryPath.empty() || util::IsLibraryAvailable(options.pylonBaseLibraryPath))) {
                if (vidre::PYLON_LIBRARY_PATH.empty()) {
                    LogMessage(options, "ERROR: Pylon plugin library path is empty. Set it in config or via --pylon-library.");
                    return nullptr;
                }
                using camera_ref_t = ICamera*();
                auto create = util::GetFunction<camera_ref_t*>(vidre::PYLON_LIBRARY_PATH, "create_pylon_camera");
                if (create == nullptr) {
                    LogMessage(options, "ERROR: Failed to load Pylon camera functions");
                    return nullptr;
                }
                camera = std::unique_ptr<ICamera>(create());
                LogMessage(options, "Created Pylon camera: " + cameraName);
            } else if (vendor == "aptina" &&
                       (options.aptinaBaseLibraryPath.empty() || util::IsLibraryAvailable(options.aptinaBaseLibraryPath))) {
                if (vidre::APTINA_LIBRARY_PATH.empty()) {
                    LogMessage(options, "ERROR: Aptina plugin library path is empty. Set it in config or via --aptina-library.");
                    return nullptr;
                }
                using camera_ref_t = ICamera*();
                auto create = util::GetFunction<camera_ref_t*>(vidre::APTINA_LIBRARY_PATH, "create_aptina_camera");
                if (create == nullptr) {
                    LogMessage(options, "ERROR: Failed to load Aptina camera functions");
                    return nullptr;
                }
                camera = std::unique_ptr<ICamera>(create());
                LogMessage(options, "Created Aptina camera: " + cameraName);
            } else {
                LogMessage(options, "ERROR: Unsupported or unavailable vendor: " + vendor);
                return nullptr;
            }

            if (cameraNode["serial"] && !camera->Init(cameraNode["serial"].as<std::string>())) {
                LogMessage(options, "ERROR: Could not initialize camera with serial: "
                            + cameraNode["serial"].as<std::string>());
                return nullptr;
            }

            if (cameraNode["config_file"]) {
                if (!camera->LoadConfiguration(cameraNode["config_file"].as<std::string>().c_str())) {
                    LogMessage(options, "ERROR: Could not load config file (check path)");
                    return nullptr;
                }
            }

            if (cameraNode["trigger_in"]) {
                camera->SetTriggerIn(cameraNode["trigger_in"].as<int>());
            }
            if (cameraNode["trigger_out"]) {
                camera->SetTriggerOut(cameraNode["trigger_out"].as<int>());
            }

            return camera;
        }
    }  // namespace

    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>
    LoadSystemSetup(const std::string& setupFilepath, LoadSetupOptions& options) {
        std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras;

        if (!std::filesystem::exists(setupFilepath)) {
            LogMessage(options, "ERROR: Configuration file " + setupFilepath + " does not exist");
            return cameras;
        }

        try {
            YAML::Node config = YAML::LoadFile(setupFilepath);

            YAML::Node libraryConfig;
            if (config["libraries"] && config["libraries"].IsMap()) {
                libraryConfig = config["libraries"];
            } else if (config["library_paths"] && config["library_paths"].IsMap()) {
                libraryConfig = config["library_paths"];
            }

            ApplyPath(ReadStringByKeys(libraryConfig, {"aptina", "aptina_library", "aptina_library_path"}),
                      options.aptinaLibraryPath, options.aptinaLibraryPathCliOverride, "aptina plugin", options);
            ApplyPath(ReadStringByKeys(libraryConfig, {"pylon", "pylon_library", "pylon_library_path"}),
                      options.pylonLibraryPath, options.pylonLibraryPathCliOverride, "pylon plugin", options);
            ApplyPath(ReadStringByKeys(libraryConfig, {"aptina_base", "aptina_base_library", "aptina_base_library_path"}),
                      options.aptinaBaseLibraryPath, options.aptinaBaseLibraryPathCliOverride, "aptina base", options);
            ApplyPath(ReadStringByKeys(libraryConfig, {"pylon_base", "pylon_base_library", "pylon_base_library_path"}),
                      options.pylonBaseLibraryPath, options.pylonBaseLibraryPathCliOverride, "pylon base", options);

            ApplyPath(ReadStringByKeys(config, {"aptina_library_path", "aptina_library"}),
                      options.aptinaLibraryPath, options.aptinaLibraryPathCliOverride, "aptina plugin", options);
            ApplyPath(ReadStringByKeys(config, {"pylon_library_path", "pylon_library"}),
                      options.pylonLibraryPath, options.pylonLibraryPathCliOverride, "pylon plugin", options);
            ApplyPath(ReadStringByKeys(config, {"aptina_base_library_path", "aptina_base_library"}),
                      options.aptinaBaseLibraryPath, options.aptinaBaseLibraryPathCliOverride, "aptina base", options);
            ApplyPath(ReadStringByKeys(config, {"pylon_base_library_path", "pylon_base_library"}),
                      options.pylonBaseLibraryPath, options.pylonBaseLibraryPathCliOverride, "pylon base", options);

            for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
                if (!it->second.IsMap()) {
                    continue;
                }
                if (!it->second["type"]) {
                    continue;
                }

                if (it->second["type"].as<std::string>() == "camera") {
                    const std::string cameraName = it->first.as<std::string>();
                    cameras[cameraName] = std::make_unique<RuntimeInstance>();

                    const YAML::Node cameraNode = it->second;
                    cameras[cameraName]->setCamera(LoadCamera(cameraName, cameraNode, options));

                    if (!cameras[cameraName]->checkCamera()) {
                        LogMessage(options, "WARNING: Camera " + cameraName + " was not constructed properly");
                        continue;
                    }

                    if (it->second["storage"]) {
                        const YAML::Node storage_config = it->second["storage"];
                        const auto output_file = storage_config["output_file"].as<std::string>();
                        const auto header_format = storage_config["header"].as<std::string>();
                        const auto bits_per_pixel = storage_config["bits_per_pixel"].as<uint64_t>();
                        const auto packed = storage_config["packed"].as<uint8_t>();

                        cameras[cameraName]->createStorage(output_file, header_format, bits_per_pixel, packed);
                        LogMessage(options, "Configured storage for " + cameraName + ": " + output_file);
                    }
                }
            }
        } catch (const YAML::Exception& e) {
            LogMessage(options, std::string("YAML Error: ") + e.what());
        } catch (const std::exception& e) {
            LogMessage(options, std::string("Error loading configuration: ") + e.what());
        }

        return cameras;
    }
}  // namespace vidre
