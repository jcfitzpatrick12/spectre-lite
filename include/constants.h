#ifndef SPECTREL_CONSTANTS_H
#define SPECTREL_CONSTANTS_H

/**
 * Constant to indicate a successfull action.
 */
#define SPECTREL_SUCCESS 0

/**
 * Constant to indicate an unsuccessful action.
 */
#define SPECTREL_FAILURE 1

/**
 * Constant to indicate that no parameters are required.
 */
#define SPECTREL_NO_SIGNAL_PARAMS NULL

/**
 * Constant to indicate max length of receiver inactivity.
 */
#define SPECTREL_TIMEOUT 1e6

/**
 * The maximum gray value in the PGM format, so that each pixel
 * can be stored as one byte.
 */
#define SPECTREL_PGM_MAXVAL 255

/**
 * The number of characters in a datetime compliant with the ISO 8601 standard
 */
#define SPECTREL_NUM_CHARS_ISO_8601 20

#endif // SPECTREL_CONSTANTS_H