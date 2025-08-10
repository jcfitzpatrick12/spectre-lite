#ifndef SPECTREL_STFFT_H
#define SPECTREL_STFFT_H

#include <fftw3.h>

typedef struct
{
    size_t num_samples;
    fftw_complex *samples;
} spectrel_signal_t;

typedef struct
{
    int num_spectrums;
    spectrel_signal_t *spectrums;
} spectrel_spectrogram_t;

typedef enum
{
    CONSTANT,
    GAUSSIAN,
    HANNING,
} spectrel_window_type_t;

spectrel_signal_t make_window(const spectrel_window_type_t window_type,
                              const size_t num_samples);

spectrel_signal_t make_cosine_signal(const size_t num_samples,
                                     const double sample_rate,
                                     const double frequency,
                                     const double amplitude,
                                     const double phase);

void print_signal(const spectrel_signal_t *signal);

void free_signal(spectrel_signal_t *signal);

void free_spectrogram(spectrel_spectrogram_t *spectrum);

spectrel_signal_t make_empty_signal(const size_t num_samples);

fftw_plan make_plan(spectrel_signal_t *buffer);

spectrel_spectrogram_t stfft(fftw_plan p,
                             spectrel_signal_t *buffer,
                             const spectrel_signal_t *signal,
                             const spectrel_signal_t *window,
                             const size_t hop);

#endif // SPECTRAL_STFFT_H