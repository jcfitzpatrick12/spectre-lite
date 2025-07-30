// Compute the DFT and write the result to file.
// Then, extend it to the SciPy ShortTimeFFT.

#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>

#include "synth_signal.h"
#include "errors.h"

void print_samples(int num_samples, fftw_complex* samples)
{
    for (int n=0; n < num_samples; n++)
    {
        printf("%f + %f\n", samples[n][0], samples[n][1]);
    }
}

int main(int argc, char* argv[])
{
    int num_samples = 16;
    synth_signal_t signal_type = CONSTANT; 
    fftw_complex* samples = malloc(sizeof(fftw_complex)*num_samples);

    if (make_signal(signal_type, num_samples, samples) == FAILURE)
    {
        fprintf(stderr, "Invalid signal type.\n");
	free(samples);
        exit(1);   
    }
     
    print_samples(num_samples, samples);

    free(samples);
    return 0;
}
