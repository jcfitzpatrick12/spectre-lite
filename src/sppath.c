
#include "sppath.h"
#include "spconstants.h"
#include "sperror.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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

static char *spectrel_join(const char *dir, const char *file_name)
{
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

spectrel_file_t *
spectrel_open_file(const char *dir, const time_t *t, const char *driver)
{
    // Convert time to UTC and format as ISO 8601
    struct tm *ut_time = gmtime(t);
    char datetime[SPECTREL_NUM_CHARS_ISO_8601+1];
    int num_chars_written =
        strftime(datetime, SPECTREL_NUM_CHARS_ISO_8601+1, "%Y-%m-%dT%H:%M:%SZ", ut_time);
    if (num_chars_written != SPECTREL_NUM_CHARS_ISO_8601)
    {
        spectrel_print_error("Failed to format the current system time");
        return NULL;
    }

    // Allocate and format the filename
    const size_t num_chars_file_name =
        strlen(datetime) + strlen("_") + strlen(driver) + strlen(".cf64") + 1;
    char *file_name = malloc(num_chars_file_name * sizeof(char));
    if (!file_name)
    {
        spectrel_print_error("Memory allocation failed for the file name");
        return NULL;
    }
    int ret = snprintf(
        file_name, num_chars_file_name, "%s_%s.cf64", datetime, driver);
    if (ret < 0)
    {
        spectrel_print_error("Failed to format the file name");
        free(file_name);
        return NULL;
    }

    // Create full file path
    char *file_path = spectrel_join(dir, file_name);
    if (!file_path)
    {
        spectrel_print_error("Failed to create the full file path");
        free(file_name);
        return NULL;
    }

    // Open the file.
    FILE *file = fopen(file_path, "wb");
    if (!file)
    {
        spectrel_print_error("Failed to open %s", file_path);
        free(file_name);
        free(file_path);
        return NULL;
    }

    // Allocate and initialize the struct
    spectrel_file_t *spfile = malloc(sizeof(spectrel_file_t));
    if (!spfile)
    {
        spectrel_print_error("Memory allocation failed for spfile");
        fclose(file);
        free(file_name);
        free(file_path);
        return NULL;
    }
    spfile->file = file;
    spfile->path = strdup(file_path);

    // Clean up temporary allocations.
    free(file_name);
    free(file_path);

    return spfile;
}

void spectrel_close_file(spectrel_file_t *file)
{
    if (file)
    {
        if (file->file)
        {
            fclose(file->file);
            file->file = NULL;
        }
        if (file->path)
        {
            free(file->path);
            file->path = NULL;
        }
        free(file);
    }
}