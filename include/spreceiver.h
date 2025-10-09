#ifndef SPRECEIVER_H
#define SPRECEIVER_H

#include "spsignal.h"

/**
 * @brief A bundle of configurable receiver parameters.
 */
typedef struct
{
    double frequency;   // The center frequency, in Hz.
    double sample_rate; // The sample rate, in Hz.
    double bandwidth;   // The bandwidth, in Hz.
    double gain;        // The gain, in dB.
} spectrel_receiver_params_t;

/**
 * @brief An opaque pointer to a receiver structure.
 */
typedef struct spectrel_receiver_t *spectrel_receiver;

/**
 * @brief Release resources allocated for a receiver structure.
 *
 * This frees all the underlying memory and clears the members.
 *
 * @param receiver The receiver structure to be cleared.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_free_receiver(spectrel_receiver receiver);

/**
 * @brief Create a new receiver structure.
 * @param driver An SDR driver supported by Soapy.
 * @param params A bundle of configurable receiver parameters.
 * @return An opaque pointer to the newly initialised receiver structure.
 */
spectrel_receiver spectrel_make_receiver(const char *driver,
                                         spectrel_receiver_params_t *params);

/**
 * @brief Get the current configured parameters for a receiver.
 * @param receiver The receiver structure to be queried.
 * @param params Pointer to struct where current parameters will be written.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_get_parameters(spectrel_receiver receiver,
                            spectrel_receiver_params_t *params);

/**
 * @brief Activate a stream, to prepare for reading.
 * @param receiver A pointer to the receiver structure.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_activate_stream(spectrel_receiver receiver);

/**
 * @brief Deactivate the stream for a receiver structure.
 * @param receiver A pointer to the receiver structure.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_deactivate_stream(spectrel_receiver receiver);

/**
 * @brief Fill the buffer with samples from the receiver.
 * @param receiver A pointer to the receiver structure.
 * @param buffer A pointer to the buffer to fill with samples from the
 * receiver.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_read_stream(spectrel_receiver receiver, spectrel_signal_t *buffer);

/**
 * @brief Print properties of the receiver, and the values of it's configured
 * parameters.
 * @param receiver The receiver struct to be described.
 */
void spectrel_describe_receiver(spectrel_receiver receiver);

#endif // SPRECEIVER_H