from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "parallel": [True, False],
        "is_32bit": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "parallel": False,
        "is_32bit": False,
    }

    def requirements(self):
        self.requires("hdf5/1.10.5")
        self.requires("medfile/4.1.1")

    def build_requirements(self):
        self.test_requires("cppunit/1.15.1")

    def layout(self):
        # self.folders.generators = "conan"
        cmake_layout(self)

    def configure(self):
        if self.options.shared:
            # fPIC might have been removed in config_options(), so we use rm_safe
            self.options.rm_safe("fPIC")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["MEDCOUPLING_USE_MPI"] = self.options.parallel
        tc.cache_variables["MEDCOUPLING_USE_64BIT_IDS"] = not self.options.is_32bit
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
