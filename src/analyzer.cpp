#include "analyzer.h"
#include <iostream>

analyzer::analyzer(size_t window_size, size_t hop_size)
    : m_window_size(window_size), m_hop_size(hop_size),
      m_cfg(kiss_fftr_alloc(window_size, 0, 0, 0)) {
  m_data_in = new kiss_fft_scalar[window_size];
  m_data_out = new float *[2];
  m_data_out[0] = new float[window_size]; // phase
  m_data_out[1] = new float[window_size]; // magnitude
  m_cx_out = new kiss_fft_cpx[(window_size / 2) + 1];
}

float *analyzer::get_input_buffer() { return m_data_in; }

float **analyzer::get_output_buffer() { return m_data_out; }

void analyzer::process() {
  for (int n = 0; n < m_window_size; n++) {
    m_data_in[n] *=
        0.5f * (1.0f - cosf(2.0 * M_PI * n / (float)(m_window_size - 1)));
  }
  kiss_fftr(m_cfg, m_data_in, m_cx_out);
  for (int n = 0; n <= m_window_size / 2; n++) {
    // Turn real and imaginary components into amplitude and phase
    // Note kiss does not scale for us so have to scale by the window size
    float amplitude =
        sqrtf(pow(m_cx_out[n].r, 2) + pow(m_cx_out[n].i, 2)) / m_window_size;

    // TODO derive scaling from hop overlap ratio, currently hard coded for 1/8
    amplitude = amplitude / 3.0;

    float phase = atan2f(m_cx_out[n].i, m_cx_out[n].r);

    m_data_out[0][n] = phase;
    m_data_out[1][n] = amplitude;
  }
}
