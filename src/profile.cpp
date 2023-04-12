#include "fukah.h"
#include <iostream>

// Native C++ invocation for profiling

int main(void) {
  std::cout << "Testing" << std::endl;
  fukah dut = fukah(0);
  constexpr int num_channels = 4;
  constexpr int data_length = 2048;
  float **test_data = new float *[num_channels];
  for (int channel_idx = 0; channel_idx < num_channels; channel_idx++) {
    test_data[channel_idx] = new float[data_length];
  }
  for (int channel_idx = 0; channel_idx < num_channels; channel_idx++) {
    for (int sample_idx = 0; sample_idx < data_length; sample_idx++) {
      test_data[channel_idx][sample_idx] =
          0.5 - (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    }
  }
  for (int n = 0; n < 5; n++) {
    dut.process(test_data, num_channels / 2, data_length);
  }
  std::cout << "Done" << std::endl;
  return 0;
}