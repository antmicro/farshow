#include "farshow/streamexception.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void initStreamException(py::module &m)
{
    py::class_<farshow::StreamException>(m, "StreamException")
        .def(py::init<const std::string &, int>(), py::arg("msg"), py::arg("error_code"))
        .def("what", &farshow::StreamException::what);
}
