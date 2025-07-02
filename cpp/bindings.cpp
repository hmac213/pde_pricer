#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "models/option.h"
#include "solvers/crank_nicolson.h"

namespace py = pybind11;

PYBIND11_MODULE(option_solver_cpp, m) {
    m.doc() = "PDE Option Pricer C++ Module";
    
    m.def("solve_crank_nicolson", &solve_crank_nicolson, "Solve the PDE using the Crank-Nicolson method");
    
    // Expose Option classes
    py::class_<Option>(m, "Option")
        .def("getK", &Option::getK)
        .def("getT", &Option::getT) 
        .def("getR", &Option::getR)
        .def("getSigma", &Option::getSigma);
    
    py::class_<EuropeanCall, Option>(m, "EuropeanCall")
        .def(py::init<double, double, double, double, double>(),
             py::arg("K"), py::arg("T"), py::arg("r"), py::arg("sigma"), py::arg("q") = 0.0);
             
    py::class_<EuropeanPut, Option>(m, "EuropeanPut")
        .def(py::init<double, double, double, double, double>(),
             py::arg("K"), py::arg("T"), py::arg("r"), py::arg("sigma"), py::arg("q") = 0.0);
             
    py::class_<AmericanCall, Option>(m, "AmericanCall")
        .def(py::init<double, double, double, double, double>(),
             py::arg("K"), py::arg("T"), py::arg("r"), py::arg("sigma"), py::arg("q") = 0.0);
             
    py::class_<AmericanPut, Option>(m, "AmericanPut")
        .def(py::init<double, double, double, double, double>(),
             py::arg("K"), py::arg("T"), py::arg("r"), py::arg("sigma"), py::arg("q") = 0.0);
} 