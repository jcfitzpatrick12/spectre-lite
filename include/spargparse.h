#ifndef SPARGPARSE_H
#define SPARGPARSE_H

/**
 * @brief Structure to hold configurable parameters.
 */
typedef struct
{
    char *dir;          // -d (directory)
    char *driver;       // -r (receiver/driver)
    double frequency;   // -f (frequency)   [Hz]
    double sample_rate; // -s (sample rate) [Hz]
    double bandwidth;   // -b (bandwidth)   [Hz]
    double gain;        // -g (gain)        [dB]
    double duration;    // -T (duration)    [s]
    int window_size;    // -w (window size) [#samples]
    int window_hop;     // -h (window hop)  [#samples]
    int buffer_size;    // -B (buffer size) [#samples]
} spectrel_args_t;

/**
 * @brief Parse command line options.
 * @param argc As passed into the main function.
 * @param argv As passed into the main function.
 * @return A structure containing the parsed command line options.
 */
spectrel_args_t *spectrel_parse_args(int argc, char *argv[]);

/**
 * @brief Release resources allocated for an args structure.
 * @param args The args structure to clear.
 * @return Zero for success, or an error code on failure.
 */
int spectrel_free_args(spectrel_args_t *args);

/**
 * @brief Print parsed command line options.
 */
void spectrel_describe_args(spectrel_args_t *args);

#endif // SPARGPARSE_H