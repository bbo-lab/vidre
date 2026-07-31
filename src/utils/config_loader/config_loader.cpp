#include <utils/config_loader/config_loader.hpp>

#include <camera/camera.hpp>
#include <camera/pylon/pylon_camera.hpp>
#include <camera/aptina/aptina_camera.hpp>

#include <core/runtime/runtime.hpp>
#include <utils/dynamic_loader/dynamic_library_loader.hpp>

#include <filesystem>
#include <iostream>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace vidre {

namespace {
void logOrIgnore(const LoaderLogCallback& logMessage, const std::string& message)
{
	if (logMessage) {
		logMessage(message);
	}
}

std::unique_ptr<vidre::ICamera> loadCamera(const std::string& cameraName,
										   const YAML::Node& cameraNode,
										   LoaderOptions& options,
										   const LoaderLogCallback& logMessage)
{
	vidre::APTINA_LIBRARY_PATH = options.aptinaLibraryPath;
	vidre::PYLON_LIBRARY_PATH = options.pylonLibraryPath;

	if (!cameraNode["vendor"]) {
		logOrIgnore(logMessage, "ERROR: No vendor specified for camera " + cameraName);
		return nullptr;
	}

	std::unique_ptr<vidre::ICamera> camera;
	const std::string vendor = cameraNode["vendor"].as<std::string>();

	if (vendor == "pylon" && (options.pylonBaseLibraryPath.empty() || vidre::util::IsLibraryAvailable(options.pylonBaseLibraryPath))) {
		if (vidre::PYLON_LIBRARY_PATH.empty()) {
			logOrIgnore(logMessage, "ERROR: Pylon plugin library path is empty. Set it in config or via --pylon-library.");
			return nullptr;
		}
		using camera_ref_t = vidre::ICamera*();
		using destroy_ref_t = void(vidre::ICamera*);
		auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::PYLON_LIBRARY_PATH, "create_pylon_camera");
		auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::PYLON_LIBRARY_PATH, "destroy_pylon_camera");

		if (create == nullptr || destroy == nullptr) {
			logOrIgnore(logMessage, "ERROR: Failed to load Pylon camera functions");
			return nullptr;
		}
		camera = std::unique_ptr<vidre::ICamera>(create());
		logOrIgnore(logMessage, "Created Pylon camera: " + cameraName);
	}
	else if (vendor == "aptina" && (options.aptinaBaseLibraryPath.empty() || vidre::util::IsLibraryAvailable(options.aptinaBaseLibraryPath))) {
		if (vidre::APTINA_LIBRARY_PATH.empty()) {
			logOrIgnore(logMessage, "ERROR: Aptina plugin library path is empty. Set it in config or via --aptina-library.");
			return nullptr;
		}
		using camera_ref_t = vidre::ICamera*();
		using destroy_ref_t = void(vidre::ICamera*);
		auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::APTINA_LIBRARY_PATH, "create_aptina_camera");
		auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::APTINA_LIBRARY_PATH, "destroy_aptina_camera");

		if (create == nullptr || destroy == nullptr) {
			logOrIgnore(logMessage, "ERROR: Failed to load Aptina camera functions");
			return nullptr;
		}
		camera = std::unique_ptr<vidre::ICamera>(create());
		logOrIgnore(logMessage, "Created Aptina camera: " + cameraName);
	}
	else {
		logOrIgnore(logMessage, "ERROR: Unsupported or unavailable vendor: " + vendor);
		return nullptr;
	}

	if (cameraNode["serial"] && !camera->Init(cameraNode["serial"].as<std::string>())) {
		logOrIgnore(logMessage, "ERROR: Could not initialize camera with serial: " + cameraNode["serial"].as<std::string>());
		return nullptr;
	}

	if (cameraNode["config_file"]) {
		if (!camera->LoadConfiguration(cameraNode["config_file"].as<std::string>().c_str())) {
			logOrIgnore(logMessage, "ERROR: Could not load config file (check path)");
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

std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> LoadSystemSetup(
	const std::string& setupFilepath,
	LoaderOptions options,
	LoaderLogCallback logMessage)
{
	std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras;

	if (!std::filesystem::exists(setupFilepath)) {
		logOrIgnore(logMessage, "ERROR: Configuration file " + setupFilepath + " does not exist");
		return cameras;
	}

	try {
		YAML::Node config = YAML::LoadFile(setupFilepath);

		auto readStringByKeys = [](const YAML::Node& node, const std::vector<std::string>& keys) -> std::string {
			if (!node || !node.IsMap()) {
				return "";
			}
			for (const auto& key : keys) {
				if (node[key] && node[key].IsScalar()) {
					return node[key].as<std::string>();
				}
			}
			return "";
		};

		auto applyPath = [&](const std::string& value, std::string& destination, bool cliOverride, const char* label) {
			if (!value.empty() && !cliOverride) {
				destination = value;
				logOrIgnore(logMessage, std::string("Config library path (") + label + "): " + value);
			}
		};

		YAML::Node libraryConfig;
		if (config["libraries"] && config["libraries"].IsMap()) {
			libraryConfig = config["libraries"];
		} else if (config["library_paths"] && config["library_paths"].IsMap()) {
			libraryConfig = config["library_paths"];
		}

		applyPath(
			readStringByKeys(libraryConfig, {"aptina", "aptina_library", "aptina_library_path"}),
			options.aptinaLibraryPath,
			options.aptinaLibraryPathCliOverride,
			"aptina plugin");
		applyPath(
			readStringByKeys(libraryConfig, {"pylon", "pylon_library", "pylon_library_path"}),
			options.pylonLibraryPath,
			options.pylonLibraryPathCliOverride,
			"pylon plugin");
		applyPath(
			readStringByKeys(libraryConfig, {"aptina_base", "aptina_base_library", "aptina_base_library_path"}),
			options.aptinaBaseLibraryPath,
			options.aptinaBaseLibraryPathCliOverride,
			"aptina base");
		applyPath(
			readStringByKeys(libraryConfig, {"pylon_base", "pylon_base_library", "pylon_base_library_path"}),
			options.pylonBaseLibraryPath,
			options.pylonBaseLibraryPathCliOverride,
			"pylon base");

		applyPath(
			readStringByKeys(config, {"aptina_library_path", "aptina_library"}),
			options.aptinaLibraryPath,
			options.aptinaLibraryPathCliOverride,
			"aptina plugin");
		applyPath(
			readStringByKeys(config, {"pylon_library_path", "pylon_library"}),
			options.pylonLibraryPath,
			options.pylonLibraryPathCliOverride,
			"pylon plugin");
		applyPath(
			readStringByKeys(config, {"aptina_base_library_path", "aptina_base_library"}),
			options.aptinaBaseLibraryPath,
			options.aptinaBaseLibraryPathCliOverride,
			"aptina base");
		applyPath(
			readStringByKeys(config, {"pylon_base_library_path", "pylon_base_library"}),
			options.pylonBaseLibraryPath,
			options.pylonBaseLibraryPathCliOverride,
			"pylon base");

		for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
			if (!it->second.IsMap()) {
				continue;
			}
			if (!it->second["type"]) {
				continue;
			}

			if (it->second["type"].as<std::string>() == "camera") {
				const std::string cameraName = it->first.as<std::string>();
				cameras[cameraName] = std::make_unique<vidre::RuntimeInstance>();
				cameras[cameraName]->setCameraName(cameraName);
				cameras[cameraName]->setTransportMode(options.transportMode);
				cameras[cameraName]->setStorageEndpoint(options.storageEndpoint);

				YAML::Node cameraNode = it->second;
				cameras[cameraName]->setCamera(loadCamera(cameraName, cameraNode, options, logMessage));

				if (!cameras[cameraName]->checkCamera()) {
					logOrIgnore(logMessage, "WARNING: Camera " + cameraName + " was not constructed properly");
					continue;
				}

				std::string output_file;
				std::string header_format = "ccv";
				uint64_t bits_per_pixel = 8;
				uint8_t packed = 0;

				if (it->second["storage"] && it->second["storage"].IsMap()) {
					auto storage_config = it->second["storage"];
					if (storage_config["output_file"] && storage_config["output_file"].IsScalar()) {
						output_file = storage_config["output_file"].as<std::string>();
					}
					if (storage_config["header"] && storage_config["header"].IsScalar()) {
						header_format = storage_config["header"].as<std::string>();
					}
					if (storage_config["bits_per_pixel"] && storage_config["bits_per_pixel"].IsScalar()) {
						bits_per_pixel = storage_config["bits_per_pixel"].as<uint64_t>();
					}
					if (storage_config["packed"] && storage_config["packed"].IsScalar()) {
						packed = storage_config["packed"].as<uint8_t>();
					}
				}

				cameras[cameraName]->createStorage(output_file, header_format, bits_per_pixel, packed);
				if (!output_file.empty()) {
					logOrIgnore(logMessage, "Configured storage for " + cameraName + ": " + output_file);
				} else if (options.transportMode == TransportMode::RemoteStorage) {
					logOrIgnore(logMessage, "Configured remote frame transport for " + cameraName);
				}
			}
		}
	}
	catch (const YAML::Exception& e) {
		logOrIgnore(logMessage, std::string("YAML Error: ") + e.what());
	}
	catch (const std::exception& e) {
		logOrIgnore(logMessage, std::string("Error loading configuration: ") + e.what());
	}

	return cameras;
}

}  // namespace vidre