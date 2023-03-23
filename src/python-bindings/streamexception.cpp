#include <pybind11/pybind11.h>
#include "farshow/streamexception.hpp"

namespace py = pybind11;

void initStreamException(py::module &m){
    py::class_<farshow::StreamException>(m, "StreamException")
        .def(py::init<const std::string &, int>(),
            py::arg("msg"), py::arg("error_code"))
        .def("what", &farshow::StreamException::what);
}
