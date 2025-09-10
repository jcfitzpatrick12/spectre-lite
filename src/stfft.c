#include "stfft.h"

#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void free_signal(spectrel_signal_t *signal)
{
    fftw_free(signal->samples);
    free(signal);
}

void describe_signal(const spectrel_signal_t *signal)
{
    printf("Number of samples: %zu\n", signal->num_samples);

    // If there's no samples, early return since we don't have anything more to
    // print.
    if (!signal->num_samples)
    {
        return;
    }

    printf("Samples:\n");
    for (size_t n = 0; n < signal->num_samples; n++)
    {
        printf("  %f + %fi\n", signal->samples[n][0], signal->samples[n][1]);
    }
}

static spectrel_signal_t *make_empty_signal(const size_t num_samples)
{
    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    // Handle if the memory allocation fails.
    if (!samples)
    {
        return NULL;
    }
    spectrel_signal_t *signal = malloc(sizeof(spectrel_signal_t));

    // Handle if the memory allocation fails.
    if (!signal)
    {
        fftw_free(samples);
        return NULL;
    }

    signal->num_samples = num_samples;
    signal->samples = samples;
    return signal;
}

spectrel_signal_t *make_buffer(const size_t num_samples)
{
    return make_empty_signal(num_samples);
}

spectrel_signal_t *make_cosine_signal(const size_t num_samples,
                                      const double sample_rate,
                                      const double frequency,
                                      const double amplitude,
                                      const double phase)
{
    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    // Handle if the memory allocation failed.
    if (!samples)
    {
        return NULL;
    }

    // Initialise the sample values.
    for (size_t n = 0; n < num_samples; n++)
    {
        double arg = 2 * M_PI * (frequency / sample_rate) * (double)(n) + phase;
        samples[n][0] = amplitude * cos(arg);

        // Zero the imaginary component.
        samples[n][1] = 0;
    }

    spectrel_signal_t *signal = malloc(sizeof(spectrel_signal_t));
    signal->num_samples = num_samples;
    signal->samples = samples;
    return signal;
}

spectrel_signal_t *make_constant_signal(const size_t num_samples,
                                        const double value)
{
    fftw_complex *samples = fftw_malloc(sizeof(fftw_complex) * num_samples);

    // Handle if the memory allocation failed.
    if (!samples)
    {
        return NULL;
    }

    // Initialise the sample values.
    for (size_t n = 0; n < num_samples; n++)
    {
        samples[n][0] = value;
        samples[n][1] = 0.0;
    }

    spectrel_signal_t *signal = malloc(sizeof(spectrel_signal_t));
    signal->num_samples = num_samples;
    signal->samples = samples;
    return signal;
}

static spectrel_signal_t *make_boxcar_window(const size_t num_samples)
{
    return make_constant_signal(num_samples, 1.0);
}

spectrel_signal_t *make_window(spectrel_window_type_t window_type,
                               const size_t num_samples)
{
    spectrel_signal_t *(*signal_generator)(const size_t num_samples);

    if (window_type == BOXCAR)
    {
        signal_generator = &make_boxcar_window;
    }

    // Handle if the window type has not been implemented.
    else
    {
        return NULL;
    }

    return signal_generator(num_samples);
}

fftw_plan make_plan(spectrel_signal_t *buffer)
{
    return fftw_plan_dft_1d(buffer->num_samples,
                            buffer->samples,
                            buffer->samples,
                            FFTW_FORWARD,
                            FFTW_ESTIMATE);
}

static spectrel_spectrogram_t *
make_empty_spectrogram(const size_t num_spectrums,
                       const size_t num_samples_per_spectrum)
{
    fftw_complex *samples =
        malloc(sizeof(fftw_complex) * num_samples_per_spectrum * num_spectrums);
    double *times = malloc(sizeof(double) * num_spectrums);
    double *frequencies = malloc(sizeof(double) * num_samples_per_spectrum);

    // Handle if any of the memory allocation failed.
    if (!samples || !times || !frequencies)
    {
        free(samples);
        free(times);
        free(frequencies);
        return NULL;
    }

    spectrel_spectrogram_t *spectrogram =
        malloc(sizeof(spectrel_spectrogram_t));
    spectrogram->num_spectrums = num_spectrums;
    spectrogram->num_samples_per_spectrum = num_samples_per_spectrum;
    spectrogram->samples = samples;
    spectrogram->times = times;
    spectrogram->frequencies = frequencies;
    return spectrogram;
}

void free_spectrogram(spectrel_spectrogram_t *spectrogram)
{
    free(spectrogram->samples);
    free(spectrogram->times);
    free(spectrogram->frequencies);
    free(spectrogram);
}

static void compute_times(double *times,
                           const size_t num_spectrums,
                           const double sample_rate,
                           const size_t window_hop)
{
    for (size_t n = 0; n < num_spectrums; n++)
    {
        times[n] = (double)(n * window_hop) * (1 / sample_rate);
    }
}

static void compute_frequencies(double *frequencies,
                                 const size_t num_samples_per_spectrum,
                                 const double sample_rate)
{
    size_t M = num_samples_per_spectrum;
    for (size_t m = 0; m < M; m++)
    {
        if (m < M / 2)
        {
            frequencies[m] = ((double)m / M) * sample_rate;
        }
        else
        {
            frequencies[m] = -1 * (1 - ((double)m / M)) * sample_rate;
        }
    }
}

spectrel_spectrogram_t *stfft(const fftw_plan p,
                              spectrel_signal_t *buffer,
                              const spectrel_signal_t *window,
                              const spectrel_signal_t *signal,
                              const size_t window_hop,
                              const double sample_rate)
{

    size_t window_size = window->num_samples;
    size_t window_midpoint = window_size / 2;
    size_t buffer_size = buffer->num_samples;
    size_t signal_size = signal->num_samples;

    // The buffer must be the same size as the window.
    if (buffer_size != window_size)
    {
        return NULL;
    }

    // The window must fit within the signal.
    if (window_size > signal_size)
    {
        return NULL;
    }

    // The window size and hop must be at least one.
    if (window_size < 1 || window_hop < 1)
    {
        return NULL;
    }

    // The number of spectrums is determined by the hop and window size.
    size_t num_spectrums =
        ((signal_size - (size_t)ceil(window_size / 2)) / window_hop) + 1;

    // The number of samples in the spectrum is the same number of samples in
    // each window.
    size_t num_samples_per_spectrum = window_size;

    // Allocate memory for an empty spectrogram.
    spectrel_spectrogram_t *s =
        make_empty_spectrogram(num_spectrums, num_samples_per_spectrum);

    // Handle if the memory allocation fails
    if (!s)
    {
        return NULL;
    }

    // Assign baseband frequencies to each spectral component.
    compute_frequencies(s->frequencies, num_samples_per_spectrum, sample_rate);

    // Assign physical times to each spectrum in the spectrogram.
    compute_times(s->times, num_spectrums, sample_rate, window_hop);

    // Initialise the window such that it's mid-point is at sample index 0.
    int start = -1 * window_midpoint;
    int end = start + window_size;

    for (size_t n = 0; n < num_spectrums; n++)
    {
        for (size_t m = 0; m < window_size; m++)
        {
            // Copy the samples for the current window into the buffer.
            // (The signal is assumed to be zero if the window dangles).
            if (start + m < 0)
            {
                buffer->samples[m][0] = 0;
                buffer->samples[m][1] = 0;
            }
            else
            {
                buffer->samples[m][0] =
                    signal->samples[start + m][0] * window->samples[m][0];
                buffer->samples[m][1] =
                    signal->samples[start + m][1] * window->samples[m][1];
            }
        }

        // Execute the DFT.
        fftw_execute(p);

        // Copy the result into the spectrogram.
        memcpy(s->samples + n * num_samples_per_spectrum,
               buffer->samples,
               sizeof(fftw_complex) * buffer_size);

        // Hop the window forward.
        start += window_hop;
        end += window_hop;
    }
    return s;
}

void describe_spectrogram(const spectrel_spectrogram_t *s)
{
    printf("Number of spectrums: %zu\n", s->num_spectrums);
    printf("Number of samples per spectrum: %zu\n",
           s->num_samples_per_spectrum);

    // If there's no samples, early return since we don't have anything more to
    // print.
    if (!s->num_spectrums || !s->num_samples_per_spectrum)
    {
        return;
    }

    size_t N = s->num_spectrums;
    size_t M = s->num_samples_per_spectrum;
    for (size_t n = 0; n < N; n++)
    {
        printf("Time %.2f [s]:\n", s->times[n]);
        for (size_t m = 0; m < M; m++)
        {
            printf("  %.2f [Hz]: %.2f + %.2fi\n",
                   s->frequencies[m],
                   s->samples[n * M + m][0],
                   s->samples[n * M + m][1]);
        }
    }
}