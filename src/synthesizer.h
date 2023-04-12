#ifndef _SYNTHESIZER_
#include "kiss_fftr.h"

class synthesizer {
private:
  size_t m_window_size;
  size_t m_hop_size;
  kiss_fftr_cfg m_cfg;
  float **m_data_in;
  float *m_real_out;
  float *m_data_out;
  kiss_fft_cpx *m_cx_in;
  int m_buf_size;

public:
  synthesizer(size_t window_size, size_t hop_size);
  float *process(float **data_in);
  float **get_input_buffers();
};
#define _SYNTHESIZER_
#endif