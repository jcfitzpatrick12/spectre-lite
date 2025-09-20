
#include "spectrel.h"

#include <stdlib.h>

int exit_failure()
{
    print_error("An unexpected error occurred");
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
    spectrel_spectrogram_t *spectrogram = NULL;
    char *dir = NULL;
    char *file_path = NULL;
    int status = SPECTREL_FAILURE;

    // TODO: Specify configurable parameters via command line arguments.
    // For now, hard-code them.
    const char *name = "hackrf";
    const double frequency = 95.8e6;
    const double sample_rate = 2e6;
    const double bandwidth = 2e6;
    const double gain = 20;
    const size_t buffer_size = 1e5;
    const size_t window_size = 1024;
    const size_t window_hop = 512;
    spectrel_format_t format = SPECTREL_FORMAT_PGM;

    // Initialise the receiver.
    receiver =
        spectrel_make_receiver(name, frequency, sample_rate, bandwidth, gain);

    if (!receiver)
        goto cleanup;

    if (spectrel_activate_stream(receiver) != 0)
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

    // Stream samples into the buffer, then apply the short-time DFT.
    // TODO: Make the duration of capture configurable, keeping track
    // of elapsed time via sample counting. For now, it's hard-coded.
    for (size_t n = 0; n < 3; n++)
    {
        if (spectrel_read_stream(receiver, buffer) != 0)
            goto cleanup;
        spectrogram =
            spectrel_stfft(plan, window, buffer, window_hop, sample_rate);
        if (!spectrogram)
            goto cleanup;
    }

    // Write the latest spectrogram to file in the PGM format.
    dir = spectrel_get_dir();
    if (spectrel_make_dir(dir) != 0)
        goto cleanup;

    file_path = spectrel_join(dir, "img.pgm");
    if (!file_path)
        goto cleanup;

    spectrel_write_spectrogram(spectrogram, file_path, format);

    status = SPECTREL_SUCCESS;

cleanup:
    if (dir)
    {
        free(dir);
        dir = NULL;
    }
    if (file_path)
    {
        free(file_path);
        file_path = NULL;
    }
    spectrel_deactivate_stream(receiver);
    if (spectrogram)
    {
        spectrel_free_spectrogram(spectrogram);
        spectrogram = NULL;
    }
    if (buffer)
    {
        spectrel_free_signal(buffer);
        buffer = NULL;
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
    if (receiver)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
    }
    return (status == SPECTREL_SUCCESS) ? exit_success() : exit_failure();
}