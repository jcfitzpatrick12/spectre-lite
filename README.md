# spectre-lite
Record spectrograms with [SoapySDR](https://github.com/pothosware/SoapySDR) and the [FFTW](https://www.fftw.org/) libraries.

> :warning: This is conduit repo to explore C, and the named libraries. Eventually, I'll add them into [spectre](https://github.com/jcfitzpatrick12/spectre).

# Prerequisites

- [FFTW3](https://www.fftw.org/) development library  
  On Ubuntu/Debian, install with:
  ```bash
  sudo apt-get install libfftw3-dev
  ```

# Installation

1. Build the binary:
   ```bash
   sudo make install
   ```

2. Run the program:
   ```bash
   spectrel
   ```