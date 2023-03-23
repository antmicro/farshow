#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "cvnp/cvnp.h"
#include "farshow/framesender.hpp"

namespace py = pybind11;

void initFrameSender(py::module &m){
    py::class_<farshow::FrameSender, farshow::UdpInterface>(m, "FrameSender")
        .def(py::init<const std::string &, int, unsigned>()
            ,py::arg("client_address"),py::arg("client_port")=1100, py::arg("frame_parts_delay")=500)
        .def("sendFrame", [](farshow::FrameSender &self, py::array &a,  std::string &name, std::string &extension, std::vector<int> &encoding_params){
            cv::Mat mat = cvnp::nparray_to_mat(a);
            self.sendFrame(mat, name, extension, encoding_params);},
            py::arg("frame"), py::arg("name"), py::arg("extension")=".jpg", py::arg("encoding_params")=std::vector<int>({cv::IMWRITE_JPEG_QUALITY, 95}))
        .def_readwrite("frame_parts_delay", &farshow::FrameSender::frame_parts_delay)
        ;
}
