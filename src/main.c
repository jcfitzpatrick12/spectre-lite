#include "spectrel.h"

int exit_success()
{
    return EXIT_SUCCESS;
}

int exit_failure()
{
    fprintf(stderr, "An unexpected error has occured.\n");
    return EXIT_FAILURE;
}

typedef enum
{
    SPECTREL_STATE_FAILURE,
    SPECTREL_STATE_OK,
} spectrel_state_t;

int main(int argc, char *argv[])
{
    // Track program state.
    spectrel_state_t state = SPECTREL_STATE_OK;

    // Initialise pointers.
    spectrel_signal_t *signal = NULL;
    spectrel_signal_t *window = NULL;
    spectrel_signal_t *buffer = NULL;

    // Initialise the cosine wave.
    const size_t num_samples = 32;
    const double sample_rate = 8;
    const double amplitude = 1;
    const double frequency = 1;
    const double phase = 0;
    spectrel_cosine_params_t cosine_params = {
        sample_rate, frequency, amplitude, phase};
    signal = make_signal(num_samples, SPECTREL_COSINE_SIGNAL, &cosine_params);

    if (!signal)
    {
        state = SPECTREL_STATE_FAILURE;
        goto cleanup;
    }

    // Initialise the window.
    const size_t window_size = 8;
    const size_t window_hop = 8;
    spectrel_constant_params_t constant_params = {1};
    window =
        make_signal(window_size, SPECTREL_CONSTANT_SIGNAL, &constant_params);

    if (!window)
    {
        state = SPECTREL_STATE_FAILURE;
        goto cleanup;
    }

    // Initialise the plan.
    spectrel_plan p = make_plan(window_size);

    if (!p)
    {
        state = SPECTREL_STATE_FAILURE;
        goto cleanup;
    }

    // Execute the short-time discrete fourier transform.
    spectrel_spectrogram_t *s =
        stfft(p, window, signal, window_hop, sample_rate);

    if (!s)
    {
        state = SPECTREL_STATE_FAILURE;
        goto cleanup;
    }

    // Print spectrogram properties.
    describe_spectrogram(s);

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
        destroy_plan(p);
        p = NULL;
    }

    if (s)
    {
        free_spectrogram(s);
        s = NULL;
    }

    return (state == SPECTREL_STATE_OK) ? exit_success() : exit_failure();
}