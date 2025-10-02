#ifndef SPECTREL_FILES_H
#define SPECTREL_FILES_H

#include <stdio.h>

/**
 * @brief A supported file format for spectrograms.
 */
typedef enum
{
    SPECTREL_FORMAT_PGM
} spectrel_format_t;

/**
 * @brief Get the directory where runtime data will be written to.
 * @return The value of the SPECTREL_DATA_DIR_PATH environment variable if it's
 * set, otherwise use the present working directory.
 * @note The returned string must be freed by the caller.
 */
char *spectrel_get_dir();

/**
 * @brief Create the input directory in the file system.
 * @param dir The directory to create.
 * @return Zero if the directory was created successfully, or it already exists.
 * Non-zero on error.
 */
int spectrel_make_dir(const char *dir);

/**
 * @brief Join a directory path and a file name into a single path string.
 * @param dir The directory path.
 * @param file_name The file name to append to the directory path.
 * @return A newly allocated string containing the combined path, or NULL on
 * allocation failure.
 * @note The returned string must be freed by the caller.
 */
char *spectrel_join(const char *dir, const char *file_name);

/**
 * @brief A batch file.
 */
typedef struct
{
    FILE *file;
    char *name;
} spectrel_batch_file_t;

/**
 * @brief Open a new batch file stream, with the current system time (UTC)
 * embedded in the file name. The file name will be of the form
 *
 * <timestamp>_<driver>.cf64
 *
 * where the timestamp is ISO 8601 standard compliant.
 *
 * @param driver A SDR driver supported by Soapy.
 * @return An opaque batch file.
 *
 */
spectrel_batch_file_t *spectrel_open_batch_file(const char *dir,
                                                const char *driver,
                                                const spectrel_format_t format);

/**
 * @brief Close a batch file, and release any resources managed by it.
 *
 * @param file The batch file.
 *
 */
void spectrel_close_batch_file(spectrel_batch_file_t *batch_file);

#endif // SPECTREL_PATHS_H