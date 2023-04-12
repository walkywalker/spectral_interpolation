#include "ring_buffer_accumulate.h"
#include <cstring>
#include <iostream>

ring_buffer_accumulate::ring_buffer_accumulate(int size, int hop_size)
    : m_size(size), m_hop_size(hop_size) {
  m_data = new float[size];
  memset(m_data, 0, size * 4);
  m_size = size;
  m_rdptr = 0;
  m_wrptr = 0;
  m_space = size;
}

ring_buffer_accumulate::~ring_buffer_accumulate() { delete[] m_data; }

// Set all data to 0 and flag buffer as empty.
bool ring_buffer_accumulate::empty() {
  m_rdptr = 0;
  m_wrptr = 0;
  m_space = m_size;
  return true;
}

int ring_buffer_accumulate::read(float *data_ptr, int num_samples) {
  // If there's nothing to read or no data available, then we can't read
  // anything.
  if (data_ptr == 0 || num_samples <= 0 || m_space == m_size) {
    return 0;
  }

  int read_space = m_size - m_space;

  // If there is not enough data in buffer to fill read request dont read
  // anything
  if (num_samples > read_space) {
    return 0;
  }

  // Simultaneously keep track of how many bytes we've read and our position in
  // the outgoing buffer
  if (num_samples > m_size - m_rdptr) {
    int len = m_size - m_rdptr;
    memcpy(data_ptr, m_data + m_rdptr, len * 4);
    memcpy(data_ptr + len, m_data, (num_samples - len) * 4);
    memset(m_data + m_rdptr, 0, len * 4);
    memset(m_data, 0, (num_samples - len) * 4);
  } else {
    memcpy(data_ptr, m_data + m_rdptr, num_samples * 4);
    memset(m_data + m_rdptr, 0, num_samples * 4);
  }

  m_rdptr = (m_rdptr + num_samples) % m_size;
  m_space += num_samples;

  return num_samples;
}

// Write to the ring buffer.  Do not overwrite data that has not yet
// been read.
int ring_buffer_accumulate::write(float *data_ptr, int num_samples) {
  // If there's nothing to write or no room available, we can't write anything.
  if (data_ptr == 0 || num_samples <= 0 || m_space == 0) {
    return 0;
  }

  // If there is not enough space in buffer to do the write request dont write
  // anything
  if (num_samples > m_space) {
    return 0;
  }

  // Simultaneously keep track of how many bytes we've written and our position
  // in the incoming buffer
  if (num_samples > m_size - m_wrptr) {
    int len = m_size - m_wrptr;
    for (size_t i = 0; i < len; i++) {
      m_data[m_wrptr + i] += data_ptr[i];
    }
    for (size_t i = 0; i < (num_samples - len); i++) {
      m_data[i] += data_ptr[len + i];
    }
  } else {
    for (size_t i = 0; i < num_samples; i++) {
      m_data[m_wrptr + i] += data_ptr[i];
    }
  }

  m_wrptr = (m_wrptr + m_hop_size) % m_size;
  m_space -= m_hop_size;

  return num_samples;
}

int ring_buffer_accumulate::get_size() { return m_size; }

int ring_buffer_accumulate::get_space() { return m_space; }

int ring_buffer_accumulate::get_read_space() { return m_size - m_space; }