#ifndef SPECTREL_STFFT_H
#define SPECTREL_STFFT_H

#include <fftw3.h>
#include <stdlib.h>

/**
 * @brief A discrete, complex-valued signal.
 */
typedef struct
{
    size_t num_samples;    /** The number of samples in the signal. */
    fftw_complex *samples; /** The sample values. */
} spectrel_signal_t;

/**
 * A supported signal type.
 */
typedef enum
{
    SPECTREL_EMPTY_SIGNAL,
    SPECTREL_CONSTANT_SIGNAL,
    SPECTREL_COSINE_SIGNAL,
} spectrel_signal_type_t;

/**
 * @brief Indicate that no parameters are required.
 */
typedef struct
{

} spectrel_no_params_t;

// Indicate that no parameters are required.
#define SPECTREL_NO_PARAMS NULL

/**
 * @brief Parameters for cosine signals.
 */
typedef struct
{
    double sample_rate;
    double frequency;
    double amplitude;
    double phase;

} spectrel_cosine_params_t;

/**
 * @brief Parameters for constant signals.
 */
typedef struct
{
    double value;
} spectrel_constant_params_t;

/**
 * @brief The spectrogram of a signal in units of DFT amplitude.
 */
typedef struct
{
    size_t num_spectrums; /** The number of spectrums in the spectrogram. */
    size_t num_samples_per_spectrum; /** The number of samples in each
                                         spectrum. */
    fftw_complex *samples; /** The DFT amplitude of each spectral component,
                               stored as a flat array. */
    double *times;       /** The physical times assigned to each spectrum in the
                             spectrogram. */
    double *frequencies; /** The baseband frequencies assigned to each spectral
                             component. */
} spectrel_spectrogram_t;

/**
 * @brief Print the number of samples in a signal, and the values of each
 * sample.
 * @param signal The signal to describe.
 */
void describe_signal(const spectrel_signal_t *signal);

/**
 * @brief Frees memory used by a signal.
 * @param signal Pointer to the signal to free.
 */
void free_signal(spectrel_signal_t *signal);

/**
 * @brief Generate a discrete, complex-valued signal.
 * @param num_samples The number of samples to take.
 * @param signal_type The type of the signal to generate
 * @param params Configurable parameters for the specified signal type.
 * @return The signal.
 */
spectrel_signal_t *make_signal(const size_t num_samples,
                               const spectrel_signal_type_t signal_type,
                               void *params);

/**
 * @brief Create an empty, memory-aligned buffer for repeated in-place DFTs by
 * FFTW.
 * @param num_samples The size of the buffer.
 * @return An empty signal.
 */
spectrel_signal_t *make_buffer(const size_t num_samples);

/**
 * @brief Plan a 1D, in-place DFT on a buffer.
 *
 * The buffer should be initialised with samples only after creating the plan.
 *
 * @param buffer An empty, uninitialised buffer on which the DFT will be
 * performed.
 * @return The FFTW plan.
 */
fftw_plan make_plan(spectrel_signal_t *buffer);

/**
 * @brief Compute the short-time discrete Fourier transform of the input signal
 * using a real sliding window.
 *
 * The first window is centered at the start of the signal (index 0). The last
 * window is the final one that fits entirely within the signal.
 *
 * @param p  A pre-planned FFTW plan for in-place transforms on the buffer.
 * @param buffer An empty buffer, used for repeated in-place DFTs.
 * @param window The window function, same length as the buffer.
 * @param signal The input signal.
 * @param window_hop The number of samples the window advances per frame.
 * @param sample_rate The sample rate of the signal.
 * @return A spectrogram containing the amplitude of each spectral component.
 */
spectrel_spectrogram_t *stfft(fftw_plan p,
                              spectrel_signal_t *buffer,
                              const spectrel_signal_t *window,
                              const spectrel_signal_t *signal,
                              const size_t window_hop,
                              const double sample_rate);

/**
 * @brief Print properties of the spectrogram, and the values of each
 * sample.
 * @param signal The signal to describe.
 */
void describe_spectrogram(const spectrel_spectrogram_t *spectrogram);

/**
 * @brief Frees memory used by a spectrogram
 * @param signal Pointer to the spectrogram to free.
 */
void free_spectrogram(spectrel_spectrogram_t *spectrogram);

#endif