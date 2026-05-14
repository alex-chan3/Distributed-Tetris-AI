from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import pybind11
import os
import shutil

class BuildExt(build_ext):
    def build_extension(self, ext):
        super().build_extension(ext)

        build_dir = os.path.dirname(self.get_ext_fullpath(ext.name))
        for filename in os.listdir(build_dir):
            if filename.startswith(ext.name) and filename.endswith(".pyd"):
                src = os.path.join(build_dir, filename)
                dst = os.path.join(os.getcwd(), "engine.pyd")
                shutil.copyfile(src, dst)
                print(f"Copied {src} -> {dst}")

                break

ext_modules = [
    Extension(
        "engine",
        [
            "src/PythonBindings.cpp",
            "src/Engine.cpp",
            "src/Piece.cpp",
            "src/Board.cpp",
            "src/Game.cpp",
            "src/Spawner.cpp",
        ],
        include_dirs = [
            "include",
            pybind11.get_include(),
        ],
        language = "c++",
        extra_compile_args = ["/std:c++20", "/O2"],
        extra_link_args = ["User32.lib"],
        py_limited_api = False,
    )
]

setup(
    name = "engine",
    version = "1.0",
    ext_modules = ext_modules,
    cmdclass = {"build_ext": BuildExt},
    zip_safe = False
)