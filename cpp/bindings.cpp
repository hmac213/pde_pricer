#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include "models/option.h"
#include "solvers/crank_nicolson.h"
#include "job_queue.h"

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

    // Expose job system
    py::class_<OptionJob>(m, "OptionJob")
        .def(py::init<std::string, std::string, double, double, double, double, double, double, double>(),
            py::arg("ticker"), py::arg("option_type"), py::arg("K"), py::arg("T"),
            py::arg("current_price"), py::arg("current_option_price"), py::arg("r"), py::arg("sigma"), py::arg("q") = 0.0)
        .def_property_readonly("ticker", &OptionJob::get_ticker)
        .def_property_readonly("option_type", &OptionJob::get_option_type)
        .def_property_readonly("K", &OptionJob::get_K)
        .def_property_readonly("T", &OptionJob::get_T)
        .def_property_readonly("current_price", &OptionJob::get_current_price)
        .def_property_readonly("current_option_price", &OptionJob::get_current_option_price)
        .def_property_readonly("r", &OptionJob::get_r)
        .def_property_readonly("sigma", &OptionJob::get_sigma)
        .def_property_readonly("q", &OptionJob::get_q)
        .def_property_readonly("S_max", &OptionJob::get_S_max)
        .def_property_readonly("J", &OptionJob::get_J)
        .def_property_readonly("N", &OptionJob::get_N);

    py::class_<OptionJobResult>(m, "OptionJobResult")
        .def_readonly("ticker", &OptionJobResult::ticker)
        .def_readonly("option_type", &OptionJobResult::option_type)
        .def_readonly("K", &OptionJobResult::K)
        .def_readonly("T", &OptionJobResult::T)
        .def_readonly("current_price", &OptionJobResult::current_price)
        .def_readonly("current_option_price", &OptionJobResult::current_option_price)
        .def_readonly("fair_value", &OptionJobResult::fair_value);

    py::class_<JobQueue>(m, "JobQueue")
        .def(py::init<>())
        .def("add_or_replace_job", &JobQueue::add_or_replace_job)
        .def("run_job", &JobQueue::run_job)
        .def("size", &JobQueue::size);

    py::class_<JobQueueProcessor>(m, "JobQueueProcessor")
        .def(py::init<>())
        .def("run_batch", &JobQueueProcessor::run_batch,
            py::call_guard<py::gil_scoped_release>(),  // Release GIL for parallel processing
            "Process jobs from queue in parallel and stream results via callback");
}