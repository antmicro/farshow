#include <cvnp/cvnp.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void initStreamException(py::module &);
void initUdpInterface(py::module &);
void initFrameSender(py::module &);
void initFrameReceiver(py::module &);

PYBIND11_MODULE(farshow, m)
{
    initStreamException(m);
    initUdpInterface(m);
    initFrameSender(m);
    initFrameReceiver(m);
}
