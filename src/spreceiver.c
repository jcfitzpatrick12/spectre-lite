
#include "spreceiver.h"
#include "spconstants.h"
#include "sperror.h"
#include "spsignal.h"

#include <SoapySDR/Constants.h>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct spectrel_receiver_t
{
    SoapySDRDevice *device;
    SoapySDRStream *rx_stream;
};

int spectrel_free_receiver(spectrel_receiver receiver)
{
    if (!receiver)
    {
        return SPECTREL_SUCCESS;
    }

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

static bool is_value_in_ranges(double value,
                               const SoapySDRRange *ranges,
                               size_t range_count)
{
    for (size_t i = 0; i < range_count; i++)
    {
        SoapySDRRange range = ranges[i];
        if (range.minimum == range.maximum)
        {
            if (value == range.minimum)
            {
                return true;
            }
        }
        if (value >= range.minimum && value <= range.maximum)
        {
            return true;
        }
    }
    return false;
}

static bool is_value_in_range(double value, const SoapySDRRange *range)
{
    return value >= range->minimum && value <= range->maximum;
}

spectrel_receiver spectrel_make_receiver(const char *driver,
                                         spectrel_receiver_params_t *params)
{

    // Prepare receiver structure with safe initial values.
    spectrel_receiver receiver = malloc(sizeof(*receiver));
    if (!receiver)
    {
        spectrel_print_error("malloc failed: receiver");
        return NULL;
    }
    receiver->device = NULL;
    receiver->rx_stream = NULL;

    // Make the soapy device for the receiver.
    SoapySDRKwargs args = {};
    if (SoapySDRKwargs_set(&args, "driver", driver) != 0)
    {
        spectrel_print_error("set fail");
        spectrel_free_receiver(receiver);
        receiver = NULL;
        return NULL;
    }
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

    size_t range_size = 0;

    // Set the frequency, first checking it's in range.
    SoapySDRRange *frequency_ranges = SoapySDRDevice_getFrequencyRange(
        receiver->device, SOAPY_SDR_RX, 0, &range_size);
    if (!frequency_ranges || range_size == 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("getFrequencyRange failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }
    if (!is_value_in_ranges(params->frequency, frequency_ranges, range_size))
    {
        SoapySDR_free(frequency_ranges);
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("Invalid frequency: %lf [Hz]", params->frequency);
        return NULL;
    }
    SoapySDR_free(frequency_ranges);
    if (SoapySDRDevice_setFrequency(
            receiver->device, SOAPY_SDR_RX, 0, params->frequency, NULL) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setFrequency failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    // Set the sample rate.
    range_size = 0;
    SoapySDRRange *sample_rate_ranges = SoapySDRDevice_getSampleRateRange(
        receiver->device, SOAPY_SDR_RX, 0, &range_size);
    if (!sample_rate_ranges || range_size == 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("getSampleRateRange failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }
    if (!is_value_in_ranges(
            params->sample_rate, sample_rate_ranges, range_size))
    {
        SoapySDR_free(sample_rate_ranges);
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("Invalid sample rate: %lf [Hz]",
                             params->sample_rate);
        return NULL;
    }
    SoapySDR_free(sample_rate_ranges);

    if (SoapySDRDevice_setSampleRate(
            receiver->device, SOAPY_SDR_RX, 0, params->sample_rate) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setSampleRate failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    // Set the bandwidth.
    range_size = 0;
    SoapySDRRange *bandwidth_ranges = SoapySDRDevice_getBandwidthRange(
        receiver->device, SOAPY_SDR_RX, 0, &range_size);
    if (!bandwidth_ranges || range_size == 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("getBandwidthRange failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }
    if (!is_value_in_ranges(params->bandwidth, bandwidth_ranges, range_size))
    {
        SoapySDR_free(bandwidth_ranges);
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("Invalid bandwidth: %lf [Hz]", params->bandwidth);
        return NULL;
    }
    SoapySDR_free(bandwidth_ranges);
    if (SoapySDRDevice_setBandwidth(
            receiver->device, SOAPY_SDR_RX, 0, params->bandwidth) != 0)
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("setBandwidth failed: %s",
                             SoapySDRDevice_lastError());
        return NULL;
    }

    // Set the gain.
    SoapySDRRange gain_range =
        SoapySDRDevice_getGainRange(receiver->device, SOAPY_SDR_RX, 0);
    if (!is_value_in_range(params->gain, &gain_range))
    {
        spectrel_free_receiver(receiver);
        receiver = NULL;
        spectrel_print_error("Invalid gain: %lf [dB]", params->gain);
        return NULL;
    }
    if (SoapySDRDevice_setGain(
            receiver->device, SOAPY_SDR_RX, 0, params->gain) != 0)
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

int spectrel_get_parameters(spectrel_receiver receiver,
                            spectrel_receiver_params_t *params)
{
    if (!params)
    {
        return SPECTREL_FAILURE;
    }
    params->frequency =
        SoapySDRDevice_getFrequency(receiver->device, SOAPY_SDR_RX, 0);
    params->sample_rate =
        SoapySDRDevice_getSampleRate(receiver->device, SOAPY_SDR_RX, 0);
    params->bandwidth =
        SoapySDRDevice_getBandwidth(receiver->device, SOAPY_SDR_RX, 0);
    params->gain = SoapySDRDevice_getGain(receiver->device, SOAPY_SDR_RX, 0);
    return SPECTREL_SUCCESS;
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

void spectrel_describe_receiver(spectrel_receiver receiver)
{
    spectrel_receiver_params_t params = {};
    spectrel_get_parameters(receiver, &params);
    printf("Frequency: %.4lf [Hz]\n", params.frequency);
    printf("Sample rate: %.4lf [Hz]\n", params.sample_rate);
    printf("Bandwidth: %.4lf [Hz]\n", params.bandwidth);
    printf("Gain: %.4lf [dB]\n", params.gain);
}