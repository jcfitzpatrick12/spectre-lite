// Clean up and simplify the short-time fft implementation.
// Return "big" structs by pointer, not by value.

#include "spectrel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exit_failure(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    // Initialise the input signal.
    const size_t num_samples = 32;
    const double sample_rate = 8;
    const double frequency = 1;
    const double amplitude = 1.0;
    const double phase = 0.0;
    spectrel_signal_t *signal = make_cosine_signal(
        num_samples, sample_rate, frequency, amplitude, phase);

    if (!signal)
    {
        exit_failure("Failed to allocate memory for the input signal.");
    }

    // Initialise the window.
    const size_t window_size = 8;
    const size_t window_hop = 8;
    const spectrel_window_type_t window_type = BOXCAR;
    spectrel_signal_t *window = make_window(window_type, window_size);

    if (!window)
    {
        exit_failure("Failed to allocate memory for the window.");
    }

    // Initialise the buffer (same size as the window.)
    const size_t buffer_size = window_size;
    spectrel_signal_t *buffer = make_buffer(buffer_size);

    if (!buffer)
    {
        exit_failure("Failed to allocate memory for the buffer.");
    }

    // Plan an in-place DFT on the buffer.
    fftw_plan p = make_plan(buffer);

    // Compute the spectrogram.
    spectrel_spectrogram_t *s =
        stfft(p, buffer, window, signal, window_hop, sample_rate);

    // Print it's properties.
    describe_spectrogram(s);

    // Release memory.
    free_signal(signal);
    free_signal(window);
    free_signal(buffer);
    free_spectrogram(s);
    fftw_destroy_plan(p);

    return EXIT_SUCCESS;
}