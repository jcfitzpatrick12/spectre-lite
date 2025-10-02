#ifndef SPECTREL_RECEIVER_H
#define SPECTREL_RECEIVER_H

#include "stfft.h"

/**
 * @brief An opaque pointer to a receiver structure.
 */
typedef struct spectrel_receiver_t *spectrel_receiver;

/**
 * @brief Create a new receiver structure.
 *
 * @param driver A SDR driver supported by Soapy.
 * @param frequency The center frequency, in Hz.
 * @param sample_rate The sample rate, in Hz.
 * @param bandwidth bandwidth, in Hz.
 * @param gain The gain, in dB.
 * @return An opaque pointer to the newly initialised receiver structure.
 */
spectrel_receiver spectrel_make_receiver(const char *driver,
                                         const double frequency,
                                         const double sample_rate,
                                         const double bandwidth,
                                         const double gain);

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
 * @brief Activate a stream, to prepare for reading.
 * @param A pointer to the receiver structure.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_activate_stream(spectrel_receiver receiver);

/**
 * @brief Activate a stream, to prepare for reading.
 * @param A pointer to the receiver structure.
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

#endif // SPECTREL_DEVICE_H