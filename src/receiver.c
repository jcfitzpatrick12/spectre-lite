#include "receiver.h"
#include "constants.h"
#include "stfft.h"

#include <SoapySDR/Constants.h>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <stdio.h>
#include <stdlib.h>

struct spectrel_receiver_t
{
    SoapySDRDevice *device;
    SoapySDRStream *rx_stream;
};

int spectrel_free_receiver(spectrel_receiver receiver)
{
    if (receiver->device && receiver->rx_stream)
    {
        if (SoapySDRDevice_closeStream(receiver->device, receiver->rx_stream) !=
            0)
        {
            fprintf(
                stderr, "closeStream fail: %s\n", SoapySDRDevice_lastError());
            return SPECTREL_FAILURE;
        }
        receiver->rx_stream = NULL;
    }

    if (receiver->device)
    {
        if (SoapySDRDevice_unmake(receiver->device) != 0)
        {
            fprintf(stderr, "unmake fail: %s\n", SoapySDRDevice_lastError());
            return SPECTREL_FAILURE;
        }
        receiver->device = NULL;
    }
    free(receiver);
    return SPECTREL_SUCCESS;
}

spectrel_receiver spectrel_make_receiver(const char *name,
                                         const double frequency,
                                         const double sample_rate,
                                         const double bandwidth,
                                         const double gain)
{

    // Prepare receiver structure with safe initial values.
    spectrel_receiver receiver = malloc(sizeof(*receiver));

    if (!receiver)
    {
        fprintf(
            stderr,
            "Malloc fail: Failed to allocate memory for the receiver pointer.");
        return NULL;
    }

    receiver->device = NULL;
    receiver->rx_stream = NULL;

    // Make the soapy device for the receiver, interpreting the receiver name as
    // the device driver.
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", name);
    receiver->device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (!receiver->device)
    {
        fprintf(stderr, "make fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    // Set the configurable parameters for the soapy device.
    if (SoapySDRDevice_setFrequency(
            receiver->device, SOAPY_SDR_RX, 0, frequency, NULL) != 0)
    {
        fprintf(stderr, "setFrequency fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    if (SoapySDRDevice_setSampleRate(
            receiver->device, SOAPY_SDR_RX, 0, sample_rate) != 0)
    {
        fprintf(stderr, "setSampleRate fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    if (SoapySDRDevice_setBandwidth(
            receiver->device, SOAPY_SDR_RX, 0, bandwidth) != 0)
    {
        fprintf(stderr, "setBandwidth fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    if (SoapySDRDevice_setGain(receiver->device, SOAPY_SDR_RX, 0, gain) != 0)
    {
        fprintf(stderr, "setGain fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    // Set up the stream.
    receiver->rx_stream = SoapySDRDevice_setupStream(
        receiver->device, SOAPY_SDR_RX, SOAPY_SDR_CF64, NULL, 0, NULL);
    if (!receiver->rx_stream)
    {
        fprintf(stderr, "setupStream fail: %s\n", SoapySDRDevice_lastError());
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }

    return receiver;
}

int spectrel_activate_stream(spectrel_receiver receiver)
{
    if (SoapySDRDevice_activateStream(
            receiver->device, receiver->rx_stream, 0, 0, 0) != 0)
    {
        fprintf(
            stderr, "activateStream fail: %s\n", SoapySDRDevice_lastError());
        return SPECTREL_FAILURE;
    }
    return SPECTREL_SUCCESS;
}

int spectrel_deactivate_stream(spectrel_receiver receiver)
{
    if (SoapySDRDevice_deactivateStream(
            receiver->device, receiver->rx_stream, 0, 0) != 0)
    {
        fprintf(
            stderr, "deactivateStream fail: %s\n", SoapySDRDevice_lastError());
        return SPECTREL_FAILURE;
    }
    return SPECTREL_SUCCESS;
}

int spectrel_read_stream(spectrel_receiver receiver, spectrel_signal_t *buffer)
{
    void *buffers[] = {buffer->samples};
    int ret = SoapySDRDevice_readStream(receiver->device,
                                        receiver->rx_stream,
                                        buffers,
                                        buffer->num_samples,
                                        0,
                                        0,
                                        SPECTREL_TIMEOUT);

    return (ret > 0) ? SPECTREL_SUCCESS : SPECTREL_FAILURE;
}