#include "fukah.h"
#include <iostream>

fukah::fukah(bool python) : m_python(python) {
  for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
    m_ring_buffers[channel_idx] =
        std::make_unique<ring_buffer>(10240, m_hop_size);
    m_analyzers[channel_idx] =
        std::make_unique<analyzer>(m_window_size, m_hop_size);
  }
  for (int input_idx = 0; input_idx < m_num_inputs; input_idx++) {
    m_out_buffers[input_idx] =
        std::make_unique<ring_buffer_accumulate>(10240, m_hop_size);
    m_synthesizers[input_idx] =
        std::make_unique<synthesizer>(m_window_size, m_hop_size);
  }
}

// output data will be in data_ptr[0,1]
void fukah::process(float **data_ptr, int num_channels, int num_samples) {
  std::vector<float> before;
  std::vector<float> after;
  if (num_channels != 2) {
    std::cerr << "Only stereo audio is supported" << std::endl;
  }

  for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
    m_ring_buffers[channel_idx]->write(data_ptr[channel_idx], num_samples);
  }

  // All ring buffers have the same occupancy
  while (m_ring_buffers[0]->get_read_space() >= m_window_size) {
    for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
      m_ring_buffers[channel_idx]->read(
          m_analyzers[channel_idx]->get_input_buffer(), m_window_size);
      m_analyzers[channel_idx]->process();
    }
    for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
      m_freq_domain[channel_idx] =
          m_analyzers[channel_idx]->get_output_buffer();
    }

    py_freq_callback(m_freq_domain);

    for (int input_idx = 0; input_idx < m_num_inputs; input_idx++) {
      m_out_buffers[input_idx]->write(
          m_synthesizers[input_idx]->process(m_freq_domain[input_idx]),
          m_window_size);
    }
  }
  for (int input_idx = 0; input_idx < m_num_inputs; input_idx++) {
    if (m_out_buffers[input_idx]->get_read_space() >= num_samples) {
      m_out_buffers[input_idx]->read(data_ptr[input_idx], num_samples);
    } else {
      memset(data_ptr[input_idx], 0, sizeof(float) * num_samples);
    }
  }
  return;
}

boost::python::list fukah::py_process(boost::python::list data) {

  int size = boost::python::len(data[0]);
  assert(size > m_hop_size);
  float **float_data = new float *[m_total_channels];
  for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
    float *channel_samples = new float[size];
    for (int i = 0; i < size; i++) {
      channel_samples[i] = boost::python::extract<float>(data[channel_idx][i]);
    }
    float_data[channel_idx] = channel_samples;
  }
  process(float_data, boost::python::len(data) / 2, size);

  boost::python::list return_data;
  boost::python::list channel_list;

  for (int input_idx = 0; input_idx < m_num_inputs; input_idx++) {
    channel_list = boost::python::list();
    for (int i = 0; i < m_window_size; i++) {
      channel_list.append(float_data[input_idx][i]);
    }
    return_data.append(channel_list);
  }
  return return_data;
}

// returned freq data will be stored in first 2 indexes
void fukah::py_freq_callback(std::array<float **, m_total_channels> freq) {

  boost::python::list all_input;
  boost::python::list channel_input;
  boost::python::list input_magnitude;
  boost::python::list input_phase;
  boost::python::list output;
  boost::python::list output_magnitude;
  boost::python::list output_phase;

  for (int channel_idx = 0; channel_idx < m_total_channels; channel_idx++) {
    input_magnitude = boost::python::list();
    input_phase = boost::python::list();
    channel_input = boost::python::list();
    for (size_t n = 0; n <= m_window_size / 2; n++) {
      input_phase.append(freq[channel_idx][0][n]);
      input_magnitude.append(freq[channel_idx][1][n]);
    }
    channel_input.append(input_phase);
    channel_input.append(input_magnitude);
    all_input.append(channel_input);
  }

  output = boost::python::call<boost::python::list>(m_freq_callback.ptr(),
                                                    all_input);

  for (int input_idx = 0; input_idx < m_num_inputs; input_idx++) {
    output_phase =
        boost::python::extract<boost::python::list>(output[input_idx][0]);
    output_magnitude =
        boost::python::extract<boost::python::list>(output[input_idx][1]);

    for (size_t n = 0; n <= m_window_size / 2; n++) {
      freq[input_idx][0][n] = boost::python::extract<float>(output_phase[n]);
      freq[input_idx][1][n] =
          boost::python::extract<float>(output_magnitude[n]);
    }
  }
}

void fukah::set_freq_callback(boost::python::object callback) {
  m_freq_callback = callback;
}

int fukah::get_window_size() { return m_window_size; }

int fukah::get_hop_size() { return m_hop_size; }