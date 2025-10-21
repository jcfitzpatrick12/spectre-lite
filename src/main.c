
#include "spectrel.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int exit_failure()
{
    return SPECTREL_FAILURE;
}

int exit_success()
{
    printf("Done.\n");
    return SPECTREL_SUCCESS;
}

int main(int argc, char *argv[])
{
    // Initialise the program.
    spectrel_args_t *args = NULL;
    spectrel_receiver receiver = NULL;
    spectrel_signal_t *buffer = NULL;
    spectrel_plan plan = NULL;
    spectrel_signal_t *window = NULL;
    spectrel_file_t *file = NULL;
    spectrel_spectrogram_t *spectrogram = NULL;
    int status = SPECTREL_FAILURE;

    args = spectrel_parse_args(argc, argv);
    if (!args)
        goto cleanup;
    spectrel_describe_args(args);

    // Initialise the receiver.
    spectrel_receiver_params_t receiver_params = {.frequency = args->frequency,
                                                  .sample_rate =
                                                      args->sample_rate,
                                                  .bandwidth = args->bandwidth,
                                                  .gain = args->gain};
    receiver = spectrel_make_receiver(args->driver, &receiver_params);
    if (!receiver)
        goto cleanup;

    spectrel_describe_receiver(receiver);

    // Create a reusable buffer to read samples from the receiver into.
    buffer =
        spectrel_make_signal(args->buffer_size, SPECTREL_EMPTY_SIGNAL, NULL);
    if (!buffer)
        goto cleanup;

    // Plan the short-time DFT.
    plan = spectrel_make_plan(args->window_size);
    if (!plan)
        goto cleanup;

    // TODO: Generalise the window (right now, the boxcar window is
    // enforced).
    spectrel_constant_params_t window_params = {1.0};
    window = spectrel_make_signal(
        args->window_size, SPECTREL_CONSTANT_SIGNAL, (void *)&window_params);
    if (!window)
        goto cleanup;

    // Elapsed time is inferred by sample counting.
    size_t num_samples_elapsed = 0;
    double sample_interval = 1 / receiver_params.sample_rate;
    size_t num_samples_total = ceil(args->duration / sample_interval);

    // Open the file to dump the spectrogram to.
    time_t now = time(NULL);
    if (spectrel_make_dir(args->dir) != 0)
        goto cleanup;
    file = spectrel_open_file(args->dir, &now, args->driver);

    // Prepare to read samples.
    if (spectrel_activate_stream(receiver) != 0)
        goto cleanup;

    // Record spectrograms until the user-specified duration has elapsed.
    while (num_samples_elapsed < num_samples_total)
    {
        if (spectrogram)
        {
            spectrel_free_spectrogram(spectrogram);
            spectrogram = NULL;
        }
        if (spectrel_read_stream(receiver, buffer) != 0)
        {
            goto cleanup;
        }
        spectrogram = spectrel_stfft(plan,
                                     window,
                                     buffer,
                                     args->window_hop,
                                     receiver_params.sample_rate);
        if (!spectrogram)
        {
            goto cleanup;
        }

        // Write the spectrogram to the file
        if (spectrel_write_spectrogram(spectrogram, file) != 0)
        {
            goto cleanup;
        }

        num_samples_elapsed += args->buffer_size;
    }
    status = SPECTREL_SUCCESS;

cleanup:
    if (spectrogram)
    {
        spectrel_free_spectrogram(spectrogram);
        spectrogram = NULL;
    }
    if (file)
    {
        spectrel_close_file(file);
        file = NULL;
    }
    if (window)
    {
        spectrel_free_signal(window);
        window = NULL;
    }
    if (plan)
    {
        spectrel_free_plan(plan);
        plan = NULL;
    }
    if (buffer)
    {
        spectrel_free_signal(buffer);
        buffer = NULL;
    }
    if (receiver)
    {
        spectrel_deactivate_stream(receiver);
        spectrel_free_receiver(receiver);
        receiver = NULL;
    }
    if (args)
    {
        spectrel_free_args(args);
        args = NULL;
    }
    return (status == SPECTREL_SUCCESS) ? exit_success() : exit_failure();
}