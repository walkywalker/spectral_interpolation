#ifndef ring_acc_def

class ring_buffer_accumulate {
private:
  float *m_data;
  int m_size;
  int m_rdptr;
  int m_wrptr;
  int m_space;
  int m_hop_size;

public:
  ring_buffer_accumulate(int size, int hop_size);
  ~ring_buffer_accumulate();
  bool empty();
  int read(float *data_ptr, int num_samples);
  int write(float *data_ptr, int num_samples);
  int get_size();
  int get_space();
  int get_read_space();
};
#define ring_acc_def
#endif
