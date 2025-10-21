#include "spargparse.h"
#include "spconstants.h"
#include "sperror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char *optarg;
extern int optind;

static void spectrel_print_usage(char *argv[])
{
    fprintf(stderr,
            "Usage: %s -r <receiver> -f <frequency> -s <sample_rate> -b "
            "<bandwidth> -g <gain> -T <duration> [-d directory] [-w "
            "window_size] [-h window_hop] [-B buffer_size]\n",
            argv[0]);
}

spectrel_args_t *spectrel_parse_args(int argc, char *argv[])
{
    spectrel_args_t *args = malloc(1 * sizeof(spectrel_args_t));
    if (!args)
        return NULL;

    // Set defaults
    args->window_size = SPECTREL_DEFAULT_WINDOW_SIZE;
    args->window_hop = SPECTREL_DEFAULT_WINDOW_HOP;
    args->buffer_size = SPECTREL_DEFAULT_BUFFER_SIZE;
    args->dir = strdup(SPECTREL_DEFAULT_DIRECTORY);
    if (!args->dir)
    {
        spectrel_free_args(args);
        return NULL;
    }

    int opt;
    while ((opt = getopt(argc, argv, "d:r:f:s:b:g:T:w:h:B:")) != -1)
    {
        char *endptr;
        switch (opt)
        {
        case 'd':
            free(args->dir);
            args->dir = strdup(optarg);
            if (!args->dir)
            {
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'r':
            args->driver = strdup(optarg);
            if (!args->driver)
            {
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'f':
            args->frequency = strtod(optarg, &endptr);
            if (*endptr != '\0')
            {
                spectrel_print_error(
                    "strtod failed: Could not cast %s as double", optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 's':
            args->sample_rate = strtod(optarg, &endptr);
            if (*endptr != '\0')
            {
                spectrel_print_error(
                    "strtod failed: Could not cast %s as double", optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'b':
            args->bandwidth = strtod(optarg, &endptr);
            if (*endptr != '\0')
            {
                spectrel_print_error(
                    "strtod failed: Could not cast %s as double", optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'g':
            args->gain = strtod(optarg, &endptr);
            if (*endptr != '\0')
            {
                spectrel_print_error(
                    "strtod failed: Could not cast %s as double", optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'T':
            args->duration = strtod(optarg, &endptr);
            if (*endptr != '\0')
            {
                spectrel_print_error(
                    "strtod failed: Could not cast %s as double", optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'w':
            args->window_size = (int)strtol(optarg, &endptr, 10);
            if (*endptr != '\0')
            {
                spectrel_print_error("strtol failed: Could not cast %s as int",
                                     optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'h':
            args->window_hop = (int)strtol(optarg, &endptr, 10);
            if (*endptr != '\0')
            {
                spectrel_print_error("strtol failed: Could not cast %s as int",
                                     optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        case 'B':
            args->buffer_size = (int)strtol(optarg, &endptr, 10);
            if (*endptr != '\0')
            {
                spectrel_print_error("strtol failed: Could not cast %s as int",
                                     optarg);
                spectrel_free_args(args);
                return NULL;
            }
            break;
        default:
            spectrel_print_usage(argv);
            spectrel_free_args(args);
            return NULL;
        }
    }

    // Check required arguments
    if (!args->driver || args->frequency == 0 || args->sample_rate == 0 ||
        args->bandwidth == 0 || args->gain == 0 || args->duration == 0)
    {
        spectrel_print_usage(argv);
        spectrel_free_args(args);
        return NULL;
    }

    return args;
}

int spectrel_free_args(spectrel_args_t *args)
{
    if (args)
    {
        if (args->dir)
            free(args->dir);
        args->dir = NULL;
        if (args->driver)
            free(args->driver);
        args->driver = NULL;
        free(args);
    }
    return SPECTREL_SUCCESS;
}

void spectrel_describe_args(spectrel_args_t *args)
{
    if (!args)
        return;
    printf("Parameters: \n");
    printf("  Directory:   %s\n", args->dir);
    printf("  Receiver:    %s\n", args->driver);
    printf("  Frequency:   %.1f [Hz]\n", args->frequency);
    printf("  Sample rate: %.1f [Hz]\n", args->sample_rate);
    printf("  Bandwidth:   %.1f [Hz]\n", args->bandwidth);
    printf("  Gain:        %.1f [dB]\n", args->gain);
    printf("  Duration:    %.2f [s]\n", args->duration);
    printf("  Window size: %d [#samples]\n", args->window_size);
    printf("  Window hop:  %d [#samples]\n", args->window_hop);
    printf("  Buffer size: %d [#samples]\n", args->buffer_size);
}