# Spectral Interpolation

This project is designed as a platform for developing audio spectral interpolation algorithms.

A user specifies 2 stereo WAV files as input, these are then played with whatever frequency domain alteration the developer has implemented in real time.

The algorithm should be implemented in Python in `daw_thread.freq_callback` in `src/daw.py`, which recieves frequency domain representation of a window of both inputs at the same time offset. The STFT analysis and re-synthesis is performed by an custom module implemented in C++.

Happy experimenting!
## Getting Started

Tested using Ubuntu 20.04. Currently only Python 3.10 is supported.

### Prerequisites

The following software is required to build and run Automix:

* git
* make
* g++ >= 11.3.0
* libboost_python310
* portaudio
* numpy
* scipy
* pyaudio >= 0.2.11

System prerequisites can easily be installed using a package manager, for example `apt`:

``` bash
> sudo apt update
> sudo apt install git make g++ libboost-all-dev libasound-dev
```

Python prerequisites can easily be installed using `pip`:
``` bash
> pip install numpy scipy PyAudio==0.2.11
```

### Building
Clone the project and it's submodules, then use make to build it:

``` bash
> git clone https://github.com/walkywalker/spectral_interpolation --recurse-submodules
> cd spectral_interpolation
> make
> . configure
```
### Running

``` bash
> python3 src/daw.py <path_to_wav_file_0> <path_to_wav_file_1>
```

## Authors

* **[Matthew Walker](https://github.com/walkywalker)** (matt26782678@gmail.com) - *Initial work*

## License

This project is licensed under the GNU General Public License.

### Submodules
* [kissfft](https://github.com/mborgerding/kissfft) (C++ STFT implementation)
