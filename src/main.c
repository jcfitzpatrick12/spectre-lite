#include "spectrel.h"

#include <math.h>
#include <stdlib.h>

void exit_with_failure(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{

    // Define a cosine wave.
    const size_t num_samples = 65536;
    const double sample_rate = 4096;
    const double frequency = 1;
    const double phase = 0;
    const double amplitude = 1;

    // Define the window.
    spectrel_window_type_t window_type = CONSTANT;
    const size_t window_size = 4096;
    const size_t hop = 4096;

    // Make the window.
    spectrel_signal_t window = make_window(window_type, window_size);

    // Make the cosine wave.
    spectrel_signal_t cosine_wave = make_cosine_signal(
        num_samples, sample_rate, frequency, amplitude, phase);

    // Make an empty buffer, where'll we'll keep performing in-place DFT's.
    spectrel_signal_t buffer = make_empty_signal(window.num_samples);

    // Check the memory allocations succeeded.
    if (!window.samples || !cosine_wave.samples || !buffer.samples)
    {
        exit_with_failure("Memory allocation "
                          "failed.");
    }

    // Make a reusable plan.
    fftw_plan p = make_plan(&buffer);

    // Perform the short-time FFT.
    spectrel_spectrogram_t s = stfft(p, &buffer, &cosine_wave, &window, hop);

    // Release memory.
    free_spectrogram(&s);
    free_signal(&buffer);
    free_signal(&cosine_wave);
    free_signal(&window);
    fftw_destroy_plan(p);
}