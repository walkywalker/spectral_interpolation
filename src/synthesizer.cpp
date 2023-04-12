#include "synthesizer.h"
#include <iostream>

synthesizer::synthesizer(size_t window_size, size_t hop_size)
    : m_window_size(window_size), m_hop_size(hop_size),
      m_cfg(kiss_fftr_alloc(window_size, 1, 0, 0)) {
  m_real_out = new kiss_fft_scalar[window_size];
  m_buf_size = 10 * window_size;
  m_data_out = new float[m_buf_size];
  m_data_in = new float *[0];
  m_data_in[0] = new float[window_size]; // frequency
  m_data_in[1] = new float[window_size]; // magnitude
  m_cx_in = new kiss_fft_cpx[(window_size / 2) + 1];
}

float **synthesizer::get_input_buffers() { return m_data_in; }

float *synthesizer::process(float **data_in) {
  m_data_in = data_in;
  for (int n = 0; n <= m_window_size / 2; n++) {
    float amplitude = m_data_in[1][n];
    float out_phase = m_data_in[0][n];

    m_cx_in[n].r = amplitude * cos(out_phase);
    m_cx_in[n].i = amplitude * sin(out_phase);
  }

  // Run the inverse FFT
  kiss_fftri(m_cfg, m_cx_in, m_real_out);
  for (int n = 0; n < m_window_size; n++) {
    m_real_out[n] *=
        0.5f * (1.0f - cosf(2.0 * M_PI * n / (float)(m_window_size - 1)));
  }
  return m_real_out;
}
