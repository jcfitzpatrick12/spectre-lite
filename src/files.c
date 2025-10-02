
#include "files.h"
#include "constants.h"
#include "errors.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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
        spectrel_print_error(
            "Memory allocation failed for directory join result");
        return NULL;
    }

    strcpy(result, dir);
    result[dir_len] = '/';
    strcpy(result + dir_len + 1, file_name);

    return result;
}

spectrel_batch_file_t *spectrel_open_batch_file(const char *dir,
                                                const char *driver_name,
                                                const spectrel_format_t format)
{
    // Get the current system time in UTC and format it in the ISO 8601 format.
    time_t t = time(NULL);
    struct tm *ut_time = gmtime(&t);
    char datetime[SPECTREL_NUM_CHARS_ISO_8601 +
                  1]; // +1 to account for the null character.
    int num_chars_written = strftime(datetime,
                                     SPECTREL_NUM_CHARS_ISO_8601 + 1,
                                     "%Y-%m-%dT%H:%M:%SZ",
                                     ut_time);
    if (num_chars_written != SPECTREL_NUM_CHARS_ISO_8601)
    {
        printf("%d\n", num_chars_written);
        spectrel_print_error("Failed to format the current system time");
        return NULL;
    }

    const char *extension;
    switch (format)
    {
    case SPECTREL_FORMAT_PGM:
        extension = "pgm";
        break;
    default:
        spectrel_print_error("Unsupported format");
        return NULL;
    }

    // Format the file name, embedding the current system time.
    const size_t num_chars = SPECTREL_NUM_CHARS_ISO_8601 + strlen("_") +
                             strlen(driver_name) + 1 + strlen(extension) + 1;
    char *name = malloc(num_chars * sizeof(char));
    if (!name)
    {
        spectrel_print_error("Memory allocation failed for the file name");
        return NULL;
    }

    int ret = snprintf(name,
                       sizeof(char) * num_chars,
                       "%s_%s.%s",
                       datetime,
                       driver_name,
                       extension);
    if (ret < 0)
    {
        spectrel_print_error("Failed to format the file name");
        free(name);
        return NULL;
    }

    dir = spectrel_join(dir, name);
    if (!dir)
    {
        spectrel_print_error("Failed to create the directory name");
        free(name);
        return NULL;
    }

    // Open the file.
    FILE *file = fopen(name, "wb");

    spectrel_batch_file_t *batch_file = malloc(sizeof(*batch_file));
    if (!batch_file)
    {
        spectrel_print_error("Memory allocation failed for the batch file.");
        return NULL;
    }

    batch_file->file = file;
    batch_file->name = name;
    return batch_file;
}

void spectrel_close_batch_file(spectrel_batch_file_t *batch_file)
{
    if (batch_file)
    {
        if (batch_file->file)
        {
            fclose(batch_file->file);
            batch_file->file = NULL;
        }
        if (batch_file->name)
        {
            free(batch_file->name);
            batch_file->name = NULL;
        }
        free(batch_file);
    }
}