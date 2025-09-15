import os
import platform

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.files import copy, update_conandata



class Vidre(ConanFile):
    name = "vidre"
    settings = "os", "compiler", "build_type", "arch"
    revision_mode = "scm"

    options = {
        "ccv_header_version": ["ANY"]
    }

    default_options = {
        "ccv_header_version": "0.12"
    }

    def set_version(self):
        self.version = "1.0.0"

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("cli11/[2.3.2]")
        self.requires("fmt/11.1.1")
        # self.requires("dylib/2.2.1")

    def build_requirements(self):
        self.tool_requires("cmake/3.29.6")

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = os.path.join(self.folders.build, "generators")

        self.cpp.source.includedirs = ["src", "include"]

        self.cpp.build.bindirs = ["bin"]
        self.cpp.build.libdirs = ["lib"]

    def generate(self):
        tc = CMakeToolchain(self)
        
        tc.cache_variables["PYLON_ROOT"] = os.getenv("PYLON_ROOT")
        tc.cache_variables["PYLON_DEV_DIR"] = os.getenv("PYLON_DEV_DIR")

        version = str(self.options.get_safe("ccv_header_version", "0.12"))
        tc.variables["CCV_HEADER_VERSION_MAJOR"] = int(version.split(".")[0])
        tc.variables["CCV_HEADER_VERSION_MINOR"] = int(version.split(".")[1])

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)

        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.components.libdirs = ['lib']
        self.cpp_info.components.libs = ['vidre']
        self.cpp_info.components.includedirs = ['src', 'include']
        self.cpp_info.components.requires = ["boost::headers", "boost::program_options", "boost::filesystem"]
        self.cpp_info.components.set_property("cmake_target_name", "vidre::vidre")