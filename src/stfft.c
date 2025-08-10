#include "stfft.h"

#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SIGMA 0.25 // Gaussian width

typedef void (*signal_generator_t)(const size_t num_samples,
                                   fftw_complex *samples);

static void signal_generator_constant(const size_t num_samples,
                                      fftw_complex *samples)
{
    for (size_t n = 0; n < num_samples; n++)
    {
        samples[n][0] = 1.0;
        samples[n][1] = 0.0;
    }
}

static void signal_generator_gaussian(const size_t num_samples,
                                      fftw_complex *samples)
{
    const double center = ((double)num_samples - 1) / 2.0;
    for (size_t n = 0; n < num_samples; n++)
    {
        double x = (n - center) / (SIGMA * center);
        samples[n][0] = exp(-0.5 * pow(x, 2));
        samples[n][1] = 0.0;
    }
}

static void signal_generator_hanning(const size_t num_samples,
                                     fftw_complex *samples)
{
    for (size_t n = 0; n < num_samples; n++)
    {
        samples[n][0] =
            0.5 * (1 - cos(2 * M_PI * n / ((double)num_samples - 1)));
        samples[n][1] = 0.0;
    }
}

static spectrel_signal_t make_null_signal()
{
    return (spectrel_signal_t){0, NULL};
}

spectrel_signal_t make_window(const spectrel_window_type_t window_type,
                              const size_t num_samples)
{
    signal_generator_t signal_generator;
    switch (window_type)
    {
    case CONSTANT:
        signal_generator = &signal_generator_constant;
        break;
    case GAUSSIAN:
        signal_generator = &signal_generator_gaussian;
        break;
    case HANNING:
        signal_generator = &signal_generator_hanning;
        break;
    default:
        return make_null_signal();
    }

    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    if (!samples)
    {
        return make_null_signal();
    }

    signal_generator(num_samples, samples);

    spectrel_signal_t signal = {num_samples, samples};
    return signal;
}

void free_signal(spectrel_signal_t *signal)
{
    if (signal && signal->samples)
    {
        fftw_free(signal->samples);
        signal->samples = NULL;
        signal->num_samples = 0;
    }
}

void print_signal(const spectrel_signal_t *signal)
{
    if (signal && signal->samples)
    {
        for (size_t n = 0; n < signal->num_samples; n++)
        {
            fprintf(stdout,
                    "%f + "
                    "%fi\n",
                    signal->samples[n][0],
                    signal->samples[n][1]);
        }
    }
}

spectrel_signal_t make_cosine_signal(const size_t num_samples,
                                     const double sample_rate,
                                     const double frequency,
                                     const double amplitude,
                                     const double phase)
{
    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    if (!samples)
    {
        return make_null_signal();
    }

    for (size_t n = 0; n < num_samples; n++)
    {
        samples[n][0] = cos(2 * M_PI * (frequency / sample_rate) * n + phase);
        samples[n][1] = 0.0;
    }

    spectrel_signal_t signal = {num_samples, samples};
    return signal;
}

spectrel_signal_t make_empty_signal(const size_t num_samples)
{
    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    if (!samples)
    {
        return make_null_signal();
    }

    spectrel_signal_t signal = {num_samples, samples};
    return signal;
}

fftw_plan make_plan(spectrel_signal_t *buffer)
{
    // In-place, 1D DFT.
    return fftw_plan_dft_1d(buffer->num_samples,
                            buffer->samples,
                            buffer->samples,
                            FFTW_FORWARD,
                            FFTW_PATIENT);
}

static spectrel_spectrogram_t make_null_spectrogram()
{
    return (spectrel_spectrogram_t){0, NULL};
}

static spectrel_spectrogram_t
make_empty_spectrogram(const size_t num_spectrums, const size_t num_frequencies)
{
    spectrel_signal_t *spectrums =
        malloc(sizeof(spectrel_signal_t) * num_spectrums);

    if (!spectrums)
    {
        return make_null_spectrogram();
    }

    for (int n = 0; n < num_spectrums; n++)
    {
        spectrums[n] = make_empty_signal(num_frequencies);
        if (!spectrums[n].samples)
        {
            // Free all previously allocated signals
            for (size_t i = 0; i < n; i++)
            {
                free_signal(&spectrums[i]);
            }
            free(spectrums);
            return make_null_spectrogram();
        }
    }
    return (spectrel_spectrogram_t){num_spectrums, spectrums};
};

void free_spectrogram(spectrel_spectrogram_t *spectrogram)
{

    if (spectrogram && spectrogram->spectrums)
    {
        for (int n = 0; n < spectrogram->num_spectrums; n++)
        {
            free_signal(&spectrogram->spectrums[n]);
        }
        free(spectrogram->spectrums);
        spectrogram->spectrums = NULL;
        spectrogram->num_spectrums = 0;
    }
}

spectrel_spectrogram_t stfft(fftw_plan p,
                             spectrel_signal_t *buffer,
                             const spectrel_signal_t *signal,
                             const spectrel_signal_t *window,
                             const size_t hop)
{
    // Ensure the buffer is the same size as the window.
    if (buffer->num_samples != window->num_samples)
    {
        return make_null_spectrogram();
    }
    const size_t window_size = window->num_samples;

    // Calculate how many spectrums (time frames) will be in the spectrogram.
    // The first window is centered at the start of the signal (index 0).
    // The last window is the final one that still overlaps with the signal.
    const size_t window_midpoint = window_size / 2;
    const size_t num_spectrums =
        (size_t)ceil((signal->num_samples + window_midpoint) / hop);

    // Allocate an empty spectrogram on the heap.
    spectrel_spectrogram_t spectrogram =
        make_empty_spectrogram(num_spectrums, window_size);

    if (!spectrogram.spectrums)
    {
        return make_null_spectrogram();
    }

    int signal_index;
    for (size_t n = 0; n < num_spectrums; n++)
    {
        // Fill up the buffer.
        for (size_t m = 0; m < window_size; m++)
        {
            signal_index = m - window_midpoint + hop * n;
            if (signal_index >= 0 && signal_index < (int)signal->num_samples)
            {
                buffer->samples[m][0] =
                    signal->samples[signal_index][0] * window->samples[m][0];
                buffer->samples[m][1] =
                    signal->samples[signal_index][1] * window->samples[m][1];
            }
            else
            {
                buffer->samples[m][0] = 0.0;
                buffer->samples[m][1] = 0.0;
            }
        }

        // Compute the DFT.
        fftw_execute(p);

        // Copy over the result into the spectrogram.
        memcpy(spectrogram.spectrums[n].samples,
               buffer->samples,
               sizeof(fftw_complex) * window_size);
    }

    return spectrogram;
}