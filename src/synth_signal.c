#include "synth_signal.h"
#include "errors.h"

#include <fftw3.h>

typedef void (*signal_generator_t)(int num_samples, fftw_complex* samples);

void signal_generator_constant(int num_samples, fftw_complex* samples)
{
    for (int n = 0; n < num_samples; n++)
    {
        samples[n][0] = 1.0;
        samples[n][1] = 0.0;	
    }
}

spectre_err_t make_signal(synth_signal_t signal_type, int num_samples, fftw_complex* samples)
{
    signal_generator_t signal_generator; 
    switch (signal_type)
    {
        case 0:
	    signal_generator = &signal_generator_constant;
	    break;
	default:
	   return FAILURE;
    }
    signal_generator(num_samples, samples);
    return SUCCESS;
}

