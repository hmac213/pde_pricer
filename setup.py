from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
import pybind11

ext_modules = [
    Pybind11Extension(
        "option_solver_cpp",
        [
            "cpp/bindings.cpp",
            "cpp/models/option.cpp", 
            "cpp/solvers/crank_nicolson.cpp",
            "cpp/solvers/mesh.cpp",
            "cpp/job_queue.cpp",
        ],
        include_dirs=["cpp", pybind11.get_cmake_dir()],
        cxx_std=14,
        define_macros=[("VERSION_INFO", '"dev"')],
        # OpenMP support
        extra_compile_args=["-fopenmp"],
        extra_link_args=["-fopenmp"],
    ),
]

setup(
    name="pde_pricer",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
) 