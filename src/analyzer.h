#ifndef _ANALYZER_
#include "kiss_fftr.h"

class analyzer {
private:
  size_t m_window_size;
  size_t m_hop_size;
  kiss_fftr_cfg m_cfg;
  float *m_data_in;
  float **m_data_out;
  kiss_fft_cpx *m_cx_out;

public:
  analyzer(size_t window_size, size_t hop_size);
  void process();
  float *get_input_buffer();
  float **get_output_buffer();
};
#define _ANALYZER_
#endif