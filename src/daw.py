from scipy.io import wavfile
import fukah_module
import numpy as np
import sys
import time
import pyaudio
from multiprocessing import Process, Pipe, Queue
import copy
from dataclasses import dataclass


@dataclass
class CONSTANTS:
    hop_size: int = fukah_module.fukah.get_hop_size()
    window_size: int = fukah_module.fukah.get_window_size()
    pyaudio_frame_size: int = 1024
    samplerate: int = 44100


class daw_thread(Process):
    def __init__(self, audio_in, frame_count, queue):
        super().__init__(daemon=True)
        self.audio_in = audio_in
        self.frame_count = frame_count
        self.queue = queue
        self.num_inputs = len(audio_in)
        self.read_ptrs = [0 for _ in self.audio_in]

    def freq_callback(self, freq_data):
        # freq_data is [input_0_phase, input_0_magnitude, input_1_phase, input_1_magnitude]
# Here is where you can try your spectral interpolation algorithms :)
# 		|\
# 		| \
# ========  -
# 		| /
# 		|/
        return freq_data[:2]
        # For now just return input_0 un-altered

    def run(self):
        full = False
        self.fukah = fukah_module.fukah()  # Must be done here rather than constructor
        self.fukah.set_freq_callback(self.freq_callback)

        while True:
            if not self.queue.full():
                data = self.num_inputs * [None]
                in_float_data = self.num_inputs * [None]
                for input_idx in range(self.num_inputs):
                    if self.read_ptrs[input_idx] + self.frame_count > len(
                        self.audio_in[input_idx]
                    ):
                        data[input_idx] = np.concatenate(
                            (
                                self.audio_in[input_idx][self.read_ptrs[input_idx] :],
                                np.zeros(
                                    (
                                        self.read_ptrs[input_idx]
                                        + self.frame_count
                                        - len(self.audio_in[input_idx])
                                    ),
                                    dtype=self.audio_in[input_idx].dtype,
                                ),
                            )
                        )
                        self.read_ptrs[input_idx] = 0
                    else:
                        data[input_idx] = self.audio_in[input_idx][
                            self.read_ptrs[input_idx] : (
                                self.read_ptrs[input_idx] + self.frame_count
                            )
                        ]
                        self.read_ptrs[input_idx] += self.frame_count

                    if data[input_idx].dtype == "int32":
                        in_float_data[input_idx] = [
                            float(n) / ((1 << 31) - 1) if n != 0 else 0
                            for n in data[input_idx]
                        ]
                    elif data[input_idx].dtype == "int16":
                        in_float_data[input_idx] = [
                            float(n) / ((1 << 15) - 1) if n != 0 else 0
                            for n in data[input_idx]
                        ]

                time_before = time.perf_counter()

                out_float_data = self.fukah.py_process(in_float_data)

                time_after = time.perf_counter()

                out_data = [
                    [
                        out_float_data[channel_idx][sample_idx] * ((1 << 31) - 1)
                        for channel_idx in range(len(out_float_data))
                    ]
                    for sample_idx in range(len(out_float_data[0]))
                ]
                out_data = np.asarray(out_data, dtype="int32")
                self.queue.put((time_after - time_before, out_data))
            else:
                sleep_time = float(self.frame_count) / CONSTANTS.samplerate

                time.sleep(sleep_time)


class daw(object):
    def __init__(self, audio_in, sample_frequency):
        self.time = 30 # Duration of looping in seconds
        self.prof_data = []
        self.callback_times = []
        self.audio_in = audio_in
        self.num_inputs = len(audio_in)
        self.read_ptrs = [0 for _ in self.audio_in]
        self.sample_frequency = sample_frequency

        self.output_wav = True
        self.wav_output = np.empty([0, 2], dtype="int32")

        self.first = True
        self.frame_count = None
        self.out_data = None

        self.parent_conn, self.child_conn = Pipe()

        self.queue = Queue(5)
        self.thread = daw_thread(
            self.audio_in, CONSTANTS.pyaudio_frame_size, self.queue
        )

    def run(self):
        self.thread.start()
        time.sleep(0.1)

        time_increment = 0.5
        current_time = 0
        p = pyaudio.PyAudio()

        stream = p.open(
            format=pyaudio.paInt32,
            channels=2,
            rate=self.sample_frequency,
            frames_per_buffer=CONSTANTS.pyaudio_frame_size,
            output=True,
            stream_callback=self.callback,
        )

        stream.start_stream()

        while stream.is_active():
            time.sleep(time_increment)
            current_time += time_increment
            if current_time >= self.time:
                break

        print("stopping stream")
        stream.stop_stream()
        stream.close()

        self.thread.terminate()

        p.terminate()

        if self.output_wav:
            wavfile.write(
                sys.argv[1].split(".wav")[0] + "fukah_output.wav",
                CONSTANTS.samplerate,
                self.wav_output,
            )

    def callback(self, in_data, frame_count, time_info, status):
        assert frame_count == self.thread.frame_count
        self.callback_times.append(time.perf_counter())

        if self.first:
            self.first = False
            out_data = np.zeros([frame_count, 2], dtype="int32")
        else:
            time_before = time.perf_counter()
            time_taken, out_data = self.queue.get()
            time_after = time.perf_counter()
            self.prof_data.append(time_taken)

        if self.output_wav:
            self.wav_output = np.concatenate((self.wav_output, out_data))

        return (out_data, pyaudio.paContinue)


if __name__ == "__main__":
    num_inputs = len(sys.argv) - 1
    if num_inputs != 2:
        print("Error expecting 2 arguments got {}".format(len(sys.argv) - 1))
        exit()

    sample_rates = num_inputs * [None]
    audio_in = num_inputs * [None]

    for arg_idx in range(num_inputs):
        sample_rates[arg_idx], audio_in[arg_idx] = wavfile.read(sys.argv[arg_idx + 1])

    if sum([freq != CONSTANTS.samplerate for freq in sample_rates]) != 0:
        print(
            "Error both inputs must have a sample rate of 44100Hz got between {} and {}".format(
                min(sample_rates), max(sample_rates)
            )
        )
        exit()

    if sum([len(track[0]) != 2 for track in audio_in]) != 0:
        print("Error both inputs must both be stereo")

    # reshape into flat array of channels
    audio_in = [channel for track in audio_in for channel in np.transpose(track)]

    dut = daw(audio_in, sample_rates[0])
    dut.run()
    print(
        "Mean time per process call {}".format(sum(dut.prof_data) / len(dut.prof_data))
    )
    callback_intervals = [
        dut.callback_times[i + 1] - dut.callback_times[i]
        for i in range(len(dut.callback_times) - 1)
    ]
    print(
        "Mean callback interval {}".format(
            sum(callback_intervals) / len(callback_intervals)
        )
    )
