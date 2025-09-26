
#include "paths.h"
#include "constants.h"
#include "error.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char *spectrel_get_dir()
{
    char *dir = getenv("SPECTREL_DATA_DIR_PATH");
    return dir ? strdup(dir) : strdup(".");
}

int spectrel_make_dir(const char *dir)
{
    int ret = mkdir(dir, 0755);
    if (ret != 0)
    {
        // If the directory already exists, that's fine.
        if (errno == EEXIST)
        {
            return SPECTREL_SUCCESS;
        }
        else
        {
            spectrel_print_error(
                "Failed to create directory '%s': %s", dir, strerror(errno));
            return SPECTREL_FAILURE;
        }
    }
    return SPECTREL_SUCCESS;
}

char *spectrel_join(const char *dir, const char *file_name)
{
    if (!dir || !file_name)
        return NULL;

    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file_name);

    // +1 for '/' +1 for null terminator
    size_t total_len = dir_len + 1 + file_len + 1;

    char *result = malloc(total_len);
    if (!result)
    {
        spectrel_print_error("Memory allocation failed for directory join result");
        return NULL;
    }

    strcpy(result, dir);
    result[dir_len] = '/';
    strcpy(result + dir_len + 1, file_name);

    return result;
}