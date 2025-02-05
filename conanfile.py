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
    }

    default_options = {
    }

    def set_version(self):
        self.version = "1.0.0"

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("cli11/[2.3.2]")
        self.requires("fmt/11.1.1")
    def build_requirements(self):
        self.tool_requires("cmake/3.29.6")

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = os.path.join(self.folders.build, "generators")

        self.cpp.source.includedirs = ["include"]

        self.cpp.build.bindirs = ["bin"]
        self.cpp.build.libdirs = ["lib"]

    def generate(self):
        tc = CMakeToolchain(self)

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