
#include "stfft.h"
#include "constants.h"
#include "error.h"

#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

void spectrel_describe_signal(const spectrel_signal_t *signal)
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
        printf("  %f + %fi\n",
               creal(signal->samples[n]),
               cimag(signal->samples[n]));
    }
}

void spectrel_free_signal(spectrel_signal_t *signal)
{
    if (signal)
    {
        if (signal->samples)
        {
            fftw_free(signal->samples);
            signal->samples = NULL;
        }

        if (signal->num_samples != 0)
        {
            signal->num_samples = 0;
        }

        free(signal);
    }
}

// Parameterised callback to initialise the signal samples.
typedef void (*spectrel_signal_generator_t)(fftw_complex *samples,
                                            const size_t num_samples,
                                            void *params);

static void spectrel_empty_signal_generator(fftw_complex *samples,
                                            const size_t num_samples,
                                            void *params)
{
    // Noop.
    return;
}

static void spectrel_cosine_signal_generator(fftw_complex *samples,
                                             const size_t num_samples,
                                             void *params)
{
    // Use defaults if no parameters are initialised.
    spectrel_cosine_params_t default_params = {
        .amplitude = 1.0, .frequency = 1.0, .phase = 0.0, .sample_rate = 8};

    spectrel_cosine_params_t *cosine_params =
        params ? (spectrel_cosine_params_t *)params : &default_params;

    for (size_t n = 0; n < num_samples; n++)
    {
        double arg =
            2 * M_PI * (cosine_params->frequency / cosine_params->sample_rate) *
                (double)(n) +
            cosine_params->phase;
        samples[n] = cosine_params->amplitude * cos(arg) + 0 * I;
    }
}

static void spectrel_constant_signal_generator(fftw_complex *samples,
                                               const size_t num_samples,
                                               void *params)
{
    spectrel_constant_params_t default_params = {.value = 1.0};
    spectrel_constant_params_t *constant_params =
        (params) ? (spectrel_constant_params_t *)(params) : &default_params;

    for (size_t n = 0; n < num_samples; n++)
    {
        samples[n] = constant_params->value + 0 * I;
    }
}

static spectrel_signal_t *
spectrel_generate_signal(const size_t num_samples,
                         spectrel_signal_generator_t signal_generator,
                         void *params)
{
    fftw_complex *samples = fftw_malloc(sizeof(*samples) * num_samples);

    if (!samples)
    {
        spectrel_print_error("Memory allocation failed for signal samples");
        return NULL;
    }

    spectrel_signal_t *signal = malloc(sizeof(*signal));
    if (!signal)
    {
        fftw_free(samples);
        samples = NULL;
        spectrel_print_error("Memory allocation failed for signal struct");
        return NULL;
    }

    signal_generator(samples, num_samples, params);

    signal->num_samples = num_samples;
    signal->samples = samples;
    return signal;
}

spectrel_signal_t *
spectrel_make_signal(const size_t num_samples,
                     const spectrel_signal_type_t signal_type,
                     void *params)
{
    spectrel_signal_generator_t signal_generator;
    switch (signal_type)
    {
    case SPECTREL_EMPTY_SIGNAL:
        signal_generator = &spectrel_empty_signal_generator;
        break;
    case SPECTREL_COSINE_SIGNAL:
        signal_generator = &spectrel_cosine_signal_generator;
        break;
    case SPECTREL_CONSTANT_SIGNAL:
        signal_generator = &spectrel_constant_signal_generator;
        break;
    default:
        spectrel_print_error("Unrecognised signal type: %d", signal_type);
        return NULL;
    }

    return spectrel_generate_signal(num_samples, signal_generator, params);
}

static spectrel_signal_t *spectrel_make_buffer(const size_t num_samples)
{
    return spectrel_make_signal(num_samples, SPECTREL_EMPTY_SIGNAL, NULL);
}

struct spectrel_plan_t
{
    spectrel_signal_t *buffer;
    fftw_plan plan;
};

void spectrel_free_plan(spectrel_plan p)
{
    if (p)
    {
        if (p->plan)
        {
            fftw_destroy_plan(p->plan);
            p->plan = NULL;
        }

        if (p->buffer)
        {
            spectrel_free_signal(p->buffer);
            p->buffer = NULL;
        }
        free(p);
    }
}

spectrel_plan spectrel_make_plan(const size_t buffer_size)
{
    spectrel_signal_t *buffer = spectrel_make_buffer(buffer_size);
    if (!buffer)
    {
        spectrel_print_error("Failed to create buffer signal");
        return NULL;
    }

    fftw_plan p = fftw_plan_dft_1d(buffer->num_samples,
                                   buffer->samples,
                                   buffer->samples,
                                   FFTW_FORWARD,
                                   FFTW_ESTIMATE);

    if (!p)
    {
        spectrel_free_signal(buffer);
        buffer = NULL;
        spectrel_print_error("Failed to create DFT plan");
        return NULL;
    }

    struct spectrel_plan_t *spectrel_plan = malloc(sizeof(*spectrel_plan));

    if (!spectrel_plan)
    {
        fftw_destroy_plan(p);
        p = NULL;

        spectrel_free_signal(buffer);
        buffer = NULL;

        spectrel_print_error("Memory allocation failed for plan struct");
        return NULL;
    }

    spectrel_plan->buffer = buffer;
    spectrel_plan->plan = p;
    return spectrel_plan;
}

static spectrel_spectrogram_t *
spectrel_make_empty_spectrogram(const size_t num_spectrums,
                                const size_t num_samples_per_spectrum)
{
    spectrel_spectrogram_t *spectrogram = malloc(sizeof(*spectrogram));
    if (!spectrogram)
    {
        spectrel_print_error("Memory allocation failed for spectrogram struct");
        return NULL;
    }

    double *times = malloc(sizeof(*times) * num_spectrums);
    if (!times)
    {
        free(spectrogram);
        spectrogram = NULL;
        spectrel_print_error("Memory allocation failed for times array");
        return NULL;
    }

    double *frequencies =
        malloc(sizeof(*frequencies) * num_samples_per_spectrum);
    if (!frequencies)
    {

        free(spectrogram);
        spectrogram = NULL;

        free(times);
        times = NULL;
        spectrel_print_error("Memory allocation failed for frequencies array");
        return NULL;
    }

    fftw_complex *samples = fftw_malloc(
        sizeof(*samples) * num_samples_per_spectrum * num_spectrums);
    if (!samples)
    {
        free(spectrogram);
        spectrogram = NULL;

        free(times);
        times = NULL;

        free(frequencies);
        frequencies = NULL;

        spectrel_print_error("Memory allocation failed for spectrogram samples");
        return NULL;
    }

    spectrogram->num_spectrums = num_spectrums;
    spectrogram->num_samples_per_spectrum = num_samples_per_spectrum;
    spectrogram->samples = samples;
    spectrogram->times = times;
    spectrogram->frequencies = frequencies;
    return spectrogram;
}

void spectrel_free_spectrogram(spectrel_spectrogram_t *spectrogram)
{
    if (spectrogram)
    {
        if (spectrogram->samples)
        {
            fftw_free(spectrogram->samples);
            spectrogram->samples = NULL;
        }

        if (spectrogram->times)
        {
            free(spectrogram->times);
            spectrogram->times = NULL;
        }

        if (spectrogram->frequencies)
        {
            free(spectrogram->frequencies);
            spectrogram->frequencies = NULL;
        }

        if (spectrogram->num_samples_per_spectrum != 0)
        {
            spectrogram->num_samples_per_spectrum = 0;
        }

        if (spectrogram->num_spectrums != 0)
        {
            spectrogram->num_spectrums = 0;
        }

        free(spectrogram);
    }
}

static void spectrel_compute_times(double *times,
                                   const size_t num_spectrums,
                                   const double sample_rate,
                                   const size_t window_hop)
{
    for (size_t n = 0; n < num_spectrums; n++)
    {
        times[n] = (double)(n * window_hop) * (1 / sample_rate);
    }
}

static void spectrel_compute_frequencies(double *frequencies,
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

void spectrel_describe_spectrogram(const spectrel_spectrogram_t *spectrogram)
{
    printf("Number of spectrums: %zu\n", spectrogram->num_spectrums);
    printf("Number of samples per spectrum: %zu\n",
           spectrogram->num_samples_per_spectrum);

    // If there's no samples, early return since we don't have anything more to
    // print.
    if (!spectrogram->num_spectrums || !spectrogram->num_samples_per_spectrum)
    {
        return;
    }

    size_t N = spectrogram->num_spectrums;
    size_t M = spectrogram->num_samples_per_spectrum;
    for (size_t n = 0; n < N; n++)
    {
        printf("Time %.2f [s]:\n", spectrogram->times[n]);
        for (size_t m = 0; m < M; m++)
        {
            printf("  %.2f [Hz]: %.2f + %.2fi\n",
                   spectrogram->frequencies[m],
                   creal(spectrogram->samples[n * M + m]),
                   cimag(spectrogram->samples[n * M + m]));
        }
    }
}

spectrel_spectrogram_t *spectrel_stfft(spectrel_plan p,
                                       const spectrel_signal_t *window,
                                       const spectrel_signal_t *signal,
                                       const size_t window_hop,
                                       const double sample_rate)
{

    size_t window_size = window->num_samples;
    size_t window_midpoint = window_size / 2;
    size_t buffer_size = p->buffer->num_samples;
    size_t signal_size = signal->num_samples;

    if (buffer_size != window_size)
    {
        spectrel_print_error("Buffer size must match window size");
        return NULL;
    }

    if (window_size > signal_size)
    {
        spectrel_print_error("Window size must not exceed signal size");
        return NULL;
    }

    if (window_size < 1 || window_hop < 1)
    {
        spectrel_print_error("Window size and hop must be at least one");
        return NULL;
    }

    // The number of spectrums is determined by the hop and window size.
    size_t num_spectrums =
        ((signal_size - (size_t)ceil(window_size / 2)) / window_hop) + 1;

    // The number of samples in the spectrum is the same number of samples in
    // each window.
    size_t num_samples_per_spectrum = window_size;

    // Allocate memory for an empty spectrogram.
    spectrel_spectrogram_t *s = spectrel_make_empty_spectrogram(
        num_spectrums, num_samples_per_spectrum);

    // Handle if the memory allocation fails
    if (!s)
    {
        spectrel_print_error("Failed to create empty spectrogram");
        return NULL;
    }

    // Assign baseband frequencies to each spectral component.
    spectrel_compute_frequencies(
        s->frequencies, num_samples_per_spectrum, sample_rate);

    // Assign physical times to each spectrum in the spectrogram.
    spectrel_compute_times(s->times, num_spectrums, sample_rate, window_hop);

    // Initialise the window such that it's mid-point is at signal index 0.
    int signal_index = -1 * window_midpoint;

    for (size_t n = 0; n < num_spectrums; n++)
    {
        // Copy the samples for the current window into the buffer.
        // (The signal is assumed to be zero if the window dangles).
        for (size_t m = 0; m < window_size; m++)
        {
            if (signal_index < 0 || signal_index >= signal_size)
            {
                p->buffer->samples[m] = 0;
            }
            else
            {
                p->buffer->samples[m] =
                    signal->samples[signal_index] * window->samples[m];
            }

            signal_index += 1;
        }

        // Execute the DFT.
        fftw_execute(p->plan);

        // Copy the result into the spectrogram.
        memcpy(s->samples + n * num_samples_per_spectrum,
               p->buffer->samples,
               sizeof(fftw_complex) * buffer_size);

        // Reset the signal index then hop the window forward.
        signal_index = (signal_index - window_size) + window_hop;
    }
    return s;
}

typedef int (*spectrel_spectrogram_writer_t)(spectrel_spectrogram_t *s,
                                             FILE *f);

static int spectrel_spectrogram_writer_pgm(spectrel_spectrogram_t *s, FILE *f)
{
    // Write the header. The PGM formats magic number is the two characters
    // "P5". The width, height and the maximum gray value are formatted as ASCII
    // characters in decimal.
    // TODO: Resolve potential overflow here.
    const size_t height = s->num_samples_per_spectrum;
    const size_t width = s->num_spectrums;
    const size_t total_num_pixels = height * width;
    fprintf(f, "P5\n%zu %zu\n%d\n", width, height, SPECTREL_PGM_MAXVAL);

    // We assume the max gray value is less than 256, so that each pixel will
    // be stored with one byte.
    if (SPECTREL_PGM_MAXVAL > 255)
    {
        spectrel_print_error("Maximum gray value must be less than 256");
        return SPECTREL_FAILURE;
    }

    // Compute the minimum and max DFT amplitude.
    double min = INFINITY, max = -INFINITY;
    for (size_t n = 0; n < total_num_pixels; n++)
    {
        double mag = cabs(s->samples[n]);
        if (mag < min)
            min = mag;
        if (mag > max)
            max = mag;
    }

    // Normalise each pixel value between [0, SPECTREL_PGM_MAXVAL).
    // Spectrograms are stored column-major, so are written to the buffer in
    // row-major.
    unsigned char *buffer = malloc(sizeof(*buffer) * total_num_pixels);
    if (!buffer)
    {
        spectrel_print_error("Memory allocation failed for PGM buffer");
        return SPECTREL_FAILURE;
    }

    for (size_t n = 0; n < height; n++)
    {
        for (size_t m = 0; m < width; m++)
        {
            size_t index = n + m * height;
            double mag = cabs(s->samples[index]);
            buffer[n * width + m] = (unsigned char)floor(
                ((mag - min) / (max - min)) * (double)SPECTREL_PGM_MAXVAL);
        }
    }

    // Write the raster of `height` rows. Rows written first are assumed to be
    // at the top.
    fwrite(buffer, sizeof(*buffer), total_num_pixels, f);

    free(buffer);
    return SPECTREL_SUCCESS;
}

int spectrel_write_spectrogram(spectrel_spectrogram_t *s,
                               const char *file_path,
                               spectrel_format_t format)
{
    FILE *f = fopen(file_path, "wb");
    if (!f)
    {
        spectrel_print_error("Failed to open file '%s' for writing", file_path);
        return SPECTREL_FAILURE;
    }

    // Choose the writer.
    spectrel_spectrogram_writer_t writer;
    switch (format)
    {
    case SPECTREL_FORMAT_PGM:
        writer = &spectrel_spectrogram_writer_pgm;
        break;
    default:
        spectrel_print_error("Unrecognised file format requested: %d", format);
        return SPECTREL_FAILURE;
    }
    // Write the spectrogram to file in the appropriate format.
    if (writer(s, f) != SPECTREL_SUCCESS)
    {
        fclose(f);
        spectrel_print_error("Failed to write spectrogram to the file %s\n", file_path);
        return SPECTREL_FAILURE;
    }

    fclose(f);
    return SPECTREL_SUCCESS;
}