#include "spectrel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int exit_failure(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return EXIT_FAILURE;
}

int exit_success()
{
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    // Initialise program status.
    int status = EXIT_FAILURE;

    // Initialise pointers.
    spectrel_signal_t *signal = NULL;
    spectrel_signal_t *window = NULL;
    spectrel_signal_t *buffer = NULL;
    fftw_plan p = NULL;
    spectrel_spectrogram_t *s = NULL;

    // Initialise the input signal.
    const size_t num_samples = 32;
    const double sample_rate = 8;
    const double frequency = 1;
    const double amplitude = 1.0;
    const double phase = 0.0;
    signal = make_cosine_signal(
        num_samples, sample_rate, frequency, amplitude, phase);

    if (!signal)
        goto cleanup;

    // Initialise the window.
    const size_t window_size = 8;
    const size_t window_hop = 8;
    const spectrel_window_type_t window_type = BOXCAR;
    window = make_window(window_type, window_size);

    if (!window)
        goto cleanup;

    // Initialise the buffer (same size as the window.)
    const size_t buffer_size = window_size;
    buffer = make_buffer(buffer_size);

    if (!buffer)
        goto cleanup;

    // Plan an in-place DFT on the buffer.
    p = make_plan(buffer);
    if (!p)
        goto cleanup;

    // Compute the spectrogram.
    s = stfft(p, buffer, window, signal, window_hop, sample_rate);
    if (!s)
        goto cleanup;

    // Print it's properties.
    describe_spectrogram(s);

    // The program has succeeded.
    status = EXIT_SUCCESS;

cleanup:
    if (signal)
    {
        free_signal(signal);
        signal = NULL;
    }

    if (window)
    {
        free_signal(window);
        window = NULL;
    }

    if (buffer)
    {
        free_signal(buffer);
        buffer = NULL;
    }

    if (p)
    {
        fftw_destroy_plan(p);
        p = NULL;
    }

    if (s)
    {
        free_spectrogram(s);
        s = NULL;
    }

    return (!status) ? exit_success()
                     : exit_failure("An unexpected error has occured.");
}