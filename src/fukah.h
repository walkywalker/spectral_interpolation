#include "analyzer.h"
#include "ring_buffer.h"
#include "ring_buffer_accumulate.h"
#include "stddef.h"
#include "synthesizer.h"
#include <boost/python.hpp>
#include <thread>
#include <vector>

class fukah {
private:
  constexpr static size_t m_window_size = 1024;
  constexpr static size_t m_hop_size = 128;
  constexpr static int m_num_inputs = 2;     // One per stereo channel per input
  constexpr static int m_total_channels = 4; // One per stereo channel per input
  std::array<std::unique_ptr<ring_buffer>, m_total_channels> m_ring_buffers;
  std::array<std::unique_ptr<ring_buffer_accumulate>, m_num_inputs>
      m_out_buffers;
  std::array<std::unique_ptr<analyzer>, m_total_channels> m_analyzers;
  std::array<std::unique_ptr<synthesizer>, m_num_inputs> m_synthesizers;
  std::array<float **, m_total_channels> m_freq_domain; // size m_window_size/2
  boost::python::object m_freq_callback;
  bool m_python;

public:
  fukah(bool python = true);
  void process(float **data_ptr, int num_channels, int num_samples);
  boost::python::list py_process(boost::python::list data);
  void set_freq_callback(boost::python::object callback);
  void py_freq_callback(std::array<float **, m_total_channels> freq);
  static int get_window_size();
  static int get_hop_size();
};