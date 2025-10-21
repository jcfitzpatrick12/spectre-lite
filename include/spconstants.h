#ifndef SPCONSTANTS_H
#define SPCONSTANTS_H

/**
 * Constant to indicate a successful action.
 */
#define SPECTREL_SUCCESS 0

/**
 * Constant to indicate an unsuccessful action.
 */
#define SPECTREL_FAILURE 1

/**
 * Constant to indicate the max length of receiver inactivity.
 */
#define SPECTREL_TIMEOUT 1e6

/**
 * The number of characters in a datetime compliant with the ISO 8601 standard.
 */
#define SPECTREL_NUM_CHARS_ISO_8601 20

/**
 * The default device buffer format.
 */
#define SPECTREL_DEFAULT_FORMAT "CF64"

/**
 * The default window size.
 */
#define SPECTREL_DEFAULT_WINDOW_SIZE 1024

/**
 * The default window hop
 */
#define SPECTREL_DEFAULT_WINDOW_HOP 512

/**
 * The default buffer size.
 */
#define SPECTREL_DEFAULT_BUFFER_SIZE 16384

/**
 * The default directory to store runtime data.
 */
#define SPECTREL_DEFAULT_DIRECTORY "."

#endif // SPCONSTANTS_H