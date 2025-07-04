from setuptools import setup, Extension
import pybind11

cpp_args = [
    '-std=c++14',
    '-O3',             # Aggressive speed optimization
    '-ffast-math',     # Allow non-IEEE compliant math
    '-fvisibility=hidden'
]

ext_modules = [
    Extension(
        'option_solver_cpp',
        ['cpp/bindings.cpp', 'cpp/job_queue.cpp', 'cpp/models/option.cpp', 'cpp/solvers/crank_nicolson.cpp', 'cpp/solvers/mesh.cpp'],
        include_dirs=[
            pybind11.get_include(),
            'cpp'
        ],
        language='c++',
        extra_compile_args=cpp_args,
    ),
]

setup(
    name='option_solver_cpp',
    version='1.0.0',
    author='Henry McNamara',
    author_email='hmac213@ucla.edu',
    description='PDE option pricer C++ module',
    ext_modules=ext_modules,
) 