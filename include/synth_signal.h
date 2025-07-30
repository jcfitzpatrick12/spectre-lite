#ifndef SYNTH_SIGNAL_H
#define SYNTH_SIGNAL_H

#include <fftw3.h>

#include "errors.h"

typedef enum
{
    CONSTANT
} synth_signal_t;

spectre_err_t make_signal(synth_signal_t signal_type , int num_samples, fftw_complex* samples);

#endif // SYNTH_SIGNAL_H
