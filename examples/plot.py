"""A basic script to plot spectrograms recorded by Spectrel.

Usage:
    python3 examples/plot.py -f 2025-10-21T22:36:10Z_rtlsdr.cf64 -w 1024
    python3 examples/plot.py -f recordings/2025-10-21T23:17:03Z_hackrf.cf64 -w 1024
"""

import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as clr


def main() -> None:
    # Parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", type=str)
    parser.add_argument("-w", type=int)
    args = parser.parse_args()
    file_path, num_samples_per_spectrum = args.f, args.w

    # The spectrograms are stored in column (spectrum) major ordering. Each sample
    # corresponds to a complex DFT amplitude, 64 bits per component.
    samples = np.fromfile(file_path, dtype=np.complex128)
    num_spectrums = len(samples) // num_samples_per_spectrum
    spectrogram = samples.reshape(num_spectrums, num_samples_per_spectrum)
    spectrogram = np.fft.fftshift(spectrogram, axes=1).T

    # Plot the spectrogram.
    plt.figure(figsize=(10, 8))
    plt.pcolormesh(np.abs(spectrogram), cmap="gnuplot2", norm=clr.LogNorm())
    plt.axis("off")
    plt.tight_layout(pad=0)
    plt.show()


if __name__ == "__main__":
    main()
