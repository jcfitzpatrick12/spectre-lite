#ifndef SPPATH_H
#define SPPATH_H

#include <stdio.h>
#include <time.h>

/**
 * @brief Create the input directory in the file system.
 * @param dir The directory to create.
 * @return Zero if the directory was created successfully, or if it already
 * exists. Non-zero on failure.
 */
int spectrel_make_dir(const char *dir);

/**
 * @brief A file to store 64-bit complex samples in the binary format.
 */
typedef struct
{
    FILE *file;
    char *path;
} spectrel_file_t;

/**
 * @brief Open a new file stream, with the input time
 * embedded in the file name. The file will be created with path:
 *
 * <dir>/<timestamp>_<driver>.cf64
 *
 * where the timestamp is UTC and ISO 8601 standard compliant.
 *
 * @param dir The parent directory for the file.
 * @param t Elapsed time since the unix epoch.
 * @param driver An SDR driver supported by Soapy.
 * @return A file struct.
 */
spectrel_file_t *
spectrel_open_file(const char *dir, const time_t *t, const char *driver);

/**
 * @brief Close a file, and release any resources managed by it.
 * @param file The file.
 */
void spectrel_close_file(spectrel_file_t *file);

#endif // SPECTREL_PATH_H