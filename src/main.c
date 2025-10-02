
#include "spectrel.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int exit_failure()
{
    spectrel_print_error("An unexpected error occurred");
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
    spectrel_receiver receiver = NULL;
    spectrel_signal_t *buffer = NULL;
    spectrel_plan plan = NULL;
    spectrel_signal_t *window = NULL;
    char *dir = NULL;
    spectrel_spectrogram_t *spectrogram = NULL;
    spectrel_batch_file_t *batch_file = NULL;
    int status = SPECTREL_FAILURE;

    // TODO: Specify configurable parameters via command line arguments.
    // For now, hard-code them.
    const char *driver = "hackrf";
    const double frequency = 100e6;
    const double sample_rate = 2e6;
    const double bandwidth = 2e6;
    const double gain = 20;
    const size_t buffer_size = 2e4;
    const size_t window_size = 4096;
    const size_t window_hop = 2048;
    const float batch_size = 2;
    const float duration = 10;
    spectrel_format_t format = SPECTREL_FORMAT_PGM;

    // Initialise the receiver.
    receiver =
        spectrel_make_receiver(driver, frequency, sample_rate, bandwidth, gain);

    if (!receiver)
        goto cleanup;

    // Create a reusable buffer ready to read samples from the receiver into.
    buffer = spectrel_make_signal(
        buffer_size, SPECTREL_EMPTY_SIGNAL, SPECTREL_NO_SIGNAL_PARAMS);
    if (!buffer)
        goto cleanup;

    // Plan the short-time DFT.
    plan = spectrel_make_plan(window_size);
    if (!plan)
        goto cleanup;

    // Create the window.
    // TODO: Generalise the window (right now, the boxcar window is enforced).
    spectrel_constant_params_t window_params = {1.0};
    window = spectrel_make_signal(
        window_size, SPECTREL_CONSTANT_SIGNAL, (void *)&window_params);
    if (!window)
        goto cleanup;

    // Set up the directory to write data produced at runtime.
    dir = spectrel_get_dir();
    if (spectrel_make_dir(dir) != 0)
        goto cleanup;

    // Stream samples into the buffer, then apply the short-time DFT.
    // TODO: Make the duration of capture configurable, keeping track
    // of elapsed time via sample counting. For now, it's hard-coded.
    if (spectrel_activate_stream(receiver) != 0)
        goto cleanup;

    // Compute the time between each sample, assuming a constant sample rate.
    double sample_interval = 1 / sample_rate;

    // Keep a record of how many samples we've streamed per batch.
    size_t num_samples_elapsed = 0;
    size_t num_samples_per_batch = ceil(batch_size / sample_interval);

    // Keep a record of how many samples we've streamed in total.
    size_t total_num_samples_elapsed = 0;
    size_t total_num_samples = ceil(duration / sample_interval);

    // Record spectrograms in batched files until the user-specified duration
    // has elapsed. Elapsed time is inferred by sample counting.
    while (total_num_samples_elapsed < total_num_samples)
    {
        if (!batch_file)
        {
            batch_file = spectrel_open_batch_file(dir, driver, format);
        }
        if (spectrogram)
        {
            spectrel_free_spectrogram(spectrogram);
            spectrogram = NULL;
        }
        if (spectrel_read_stream(receiver, buffer) != 0)
        {
            goto cleanup;
        }
        spectrogram =
            spectrel_stfft(plan, window, buffer, window_hop, sample_rate);
        if (!spectrogram)
        {
            goto cleanup;
        }

        // Write the spectrogram to the batch file
        if (spectrel_write_spectrogram(spectrogram, batch_file, format) != 0)
        {
            goto cleanup;
        }

        total_num_samples_elapsed += buffer_size;
        num_samples_elapsed += buffer_size;

        if (num_samples_elapsed > num_samples_per_batch)
        {
            spectrel_close_batch_file(batch_file);
            batch_file = NULL;
            num_samples_elapsed = 0;
        }
    }

    status = SPECTREL_SUCCESS;

cleanup:
    if (batch_file)
    {
        spectrel_close_batch_file(batch_file);
        batch_file = NULL;
    }
    if (dir)
    {
        free(dir);
        dir = NULL;
    }
    if (spectrogram)
    {
        spectrel_free_spectrogram(spectrogram);
        spectrogram = NULL;
    }
    spectrel_deactivate_stream(receiver);
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
        spectrel_free_receiver(receiver);
        receiver = NULL;
    }
    return (status == SPECTREL_SUCCESS) ? exit_success() : exit_failure();
}