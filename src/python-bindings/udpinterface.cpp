#include "farshow/udpinterface.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;
void initUdpInterface(py::module &m)
{
    py::class_<farshow::FrameHeader>(m, "FrameHeader")
        .def(py::init<unsigned, unsigned, unsigned, unsigned>(), py::arg("name_length"), py::arg("frame_id"),
             py::arg("part_id"), py::arg("total_parts"))
        .def_readwrite("name_length", &farshow::FrameHeader::name_length)
        .def_readwrite("frame_id", &farshow::FrameHeader::frame_id)
        .def_readwrite("part_id", &farshow::FrameHeader::part_id)
        .def_readwrite("total_parts", &farshow::FrameHeader::total_parts);
    py::class_<farshow::FrameMessage>(m, "FrameMessage")
        .def(py::init(
                 [](farshow::FrameHeader header, std::string &data)
                 {
                     farshow::FrameMessage msg = {header, ""};
                     std::strcpy(msg.data, data.c_str());
                     return msg;
                 }),
             py::arg("header"), py::arg("data"))
        .def_readwrite("header", &farshow::FrameMessage::header)
        .def_property(
            "data", [](farshow::FrameMessage &self) { return self.data; },
            [](farshow::FrameMessage &self, std::string &a) { std::strcpy(self.data, a.c_str()); });
    py::class_<farshow::UdpInterface>(m, "UdpInterface")
        .def(py::init<const std::string &, int>(), py::arg("client_address") = "", py::arg("client_port") = 1100);
}
