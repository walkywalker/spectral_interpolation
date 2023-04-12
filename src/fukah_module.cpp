#include "fukah.h"
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(fukah_module) {
  class_<fukah, boost::noncopyable>("fukah", init<>())
      .def("py_process", &fukah::py_process)
      .def("set_freq_callback", &fukah::set_freq_callback)
      .def("get_hop_size", &fukah::get_hop_size)
      .def("get_window_size", &fukah::get_window_size);
}
