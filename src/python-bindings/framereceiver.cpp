#include "farshow/framereceiver.hpp"
#include "cvnp/cvnp.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<uchar>)

void initFrameReceiver(py::module &m)
{
    py::bind_vector<std::vector<uchar>>(m, "VectorUchar");
    py::class_<farshow::Frame>(m, "Frame")
        .def(py::init(
                 [](const std::string &name, py::array &a) {
                     return farshow::Frame{name, cvnp::nparray_to_mat(a)};
                 }),
             py::arg("name"), py::arg("img"))
        .def_readwrite("name", &farshow::Frame::name)
        .def_property(
            "img", [](farshow::Frame &self) { return cvnp::mat_to_nparray(self.img, true); },
            [](farshow::Frame &self, py::array &a) { self.img = cvnp::nparray_to_mat(a); });
    py::class_<farshow::FrameContainer>(m, "FrameContainer")
        .def(py::init<unsigned, unsigned, const std::string &, unsigned>(), py::arg("id"), py::arg("total_parts"),
             py::arg("name"), py::arg("frame_size"))
        .def("isComplete", &farshow::FrameContainer::isComplete)
        .def_readwrite("id", &farshow::FrameContainer::id)
        .def_readwrite("total_parts", &farshow::FrameContainer::total_parts)
        .def_readwrite("added_parts", &farshow::FrameContainer::added_parts)
        .def_readwrite("img", &farshow::FrameContainer::img)
        .def_readwrite("name", &farshow::FrameContainer::name);
    py::class_<farshow::FrameReceiver>(m, "FrameReceiver")
        .def(py::init<const std::string &, int>(), py::arg("client_address") = "", py::arg("client_port") = 1100)
        .def("receiveFrame", &farshow::FrameReceiver::receiveFrame)
        .def("getSocket", &farshow::FrameReceiver::getSocket);
}
