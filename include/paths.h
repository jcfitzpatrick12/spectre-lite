#ifndef SPECTREL_PATHS_H
#define SPECTREL_PATHS_H

/**
 * @brief Get the directory where runtime data will be written to.
 * @return The value of the SPECTREL_DATA_DIR_PATH environment variable if it's
 * set, otherwise use the present working directory.
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

#endif // SPECTREL_PATHS_H