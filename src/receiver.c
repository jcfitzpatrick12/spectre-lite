
#include "receiver.h"
#include "constants.h"
#include "errors.h"
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
            spectrel_print_error("closeStream failed: %s",
                                 SoapySDRDevice_lastError());
            return SPECTREL_FAILURE;
        }
        receiver->rx_stream = NULL;
    }

    if (receiver->device)
    {
        if (SoapySDRDevice_unmake(receiver->device) != 0)
        {
            spectrel_print_error("unmake failed: %s",
                                 SoapySDRDevice_lastError());
            return SPECTREL_FAILURE;
        }
        receiver->device = NULL;
    }
    free(receiver);
    return SPECTREL_SUCCESS;
}

spectrel_receiver spectrel_make_receiver(const char *driver,
                                         const double frequency,
                                         const double sample_rate,
                                         const double bandwidth,
                                         const double gain)
{

    // Prepare receiver structure with safe initial values.
    spectrel_receiver receiver = malloc(sizeof(*receiver));

    if (!receiver)
    {
        spectrel_print_error("Memory allocation failed for receiver");
        return NULL;
    }

    receiver->device = NULL;
    receiver->rx_stream = NULL;

    // Make the soapy device for the receiver.
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", driver);
    receiver->device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (!receiver->device)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("Device creation failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    // Set the configurable parameters for the soapy device.
    if (SoapySDRDevice_setFrequency(
            receiver->device, SOAPY_SDR_RX, 0, frequency, NULL) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setFrequency failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    if (SoapySDRDevice_setSampleRate(
            receiver->device, SOAPY_SDR_RX, 0, sample_rate) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setSampleRate failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    if (SoapySDRDevice_setBandwidth(
            receiver->device, SOAPY_SDR_RX, 0, bandwidth) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setBandwidth failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    if (SoapySDRDevice_setGain(receiver->device, SOAPY_SDR_RX, 0, gain) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setGain failed: %s", SoapySDRDevice_lastError());
        return NULL;
    }

    // Set up the stream.
    receiver->rx_stream = SoapySDRDevice_setupStream(
        receiver->device, SOAPY_SDR_RX, SOAPY_SDR_CF64, NULL, 0, NULL);
    if (!receiver->rx_stream)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setupStream failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    return receiver;
}

int spectrel_activate_stream(spectrel_receiver receiver)
{
    if (SoapySDRDevice_activateStream(
            receiver->device, receiver->rx_stream, 0, 0, 0) != 0)
    {
        spectrel_print_error("activateStream failed: %s",
                             SoapySDRDevice_lastError());
        return SPECTREL_FAILURE;
    }
    return SPECTREL_SUCCESS;
}

int spectrel_deactivate_stream(spectrel_receiver receiver)
{
    if (SoapySDRDevice_deactivateStream(
            receiver->device, receiver->rx_stream, 0, 0) != 0)
    {
        spectrel_print_error("deactivateStream failed: %s",
                             SoapySDRDevice_lastError());
        return SPECTREL_FAILURE;
    }
    return SPECTREL_SUCCESS;
}

int spectrel_read_stream(spectrel_receiver receiver, spectrel_signal_t *buffer)
{
    int num_samples_read = 0;
    void *buffers[] = {NULL};

    // Keep calling readStream until the buffer is full.
    while (num_samples_read < buffer->num_samples)
    {
        buffers[0] = (void *)(buffer->samples + num_samples_read);
        int ret =
            SoapySDRDevice_readStream(receiver->device,
                                      receiver->rx_stream,
                                      buffers,
                                      buffer->num_samples - num_samples_read,
                                      0,
                                      0,
                                      SPECTREL_TIMEOUT);
        if (ret < 1)
        {
            spectrel_print_error("readStream fail: %s\n",
                                 SoapySDRDevice_lastError());
            return SPECTREL_FAILURE;
        }
        num_samples_read += ret;
    }

    return SPECTREL_SUCCESS;
}