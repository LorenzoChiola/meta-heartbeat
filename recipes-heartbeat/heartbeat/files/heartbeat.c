// Heartbeat application
// Authors: Lorenzo Chiola, Massimo Violante

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define q	11		    /* for 2^11 points */
#define N	(1<<q)		/* N-point FFT, iFFT */

typedef float real;
typedef struct{real Re; real Im;} complex;

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif


void fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

/// @param Device name is optionally passed as the first and only parameter.
int main(int argc, char* argv[])
{
	complex v[N], scratch[N];
	float abs[N];
	int k;
	int m;
	//int i;
	int minIdx, maxIdx;
	
	char dev_name[PATH_MAX] = "/dev/heartmon0"; // TODO parameter?
	char buf[64];
	int temp;
	int len;
	int heartmonfile = 0;
	
	// Get device name from command line if present
	if(argc >= 2)
		strcpy(dev_name, argv[1]);
	
	heartmonfile = open(dev_name, O_RDWR);
	if(heartmonfile < 0)
	{
		fprintf(stderr, "Heart monitoring device %s could not be opened.\n", dev_name);
		exit(EXIT_FAILURE);
	}

	while(1) // while(forever)
	{
		// Initialize the complex array for FFT computation, reading N samples from the heart monitor module
		for(k=0; k<N; k++) {
			len = read(heartmonfile, buf, 63);
			if(len > 0)
				buf[len] = '\0'; // Terminate buf string
			if(sscanf(buf, "%d", &temp) != 1)
				k--; // do not advance sample counter if we received garbage
			else
				v[k].Re = temp; // using temp automatically casts the data to the type of complex.Re
			v[k].Im = 0;
		}
		//printf("2048 numbers were read");
		
		// FFT computation
		fft( v, N, scratch );

		// PSD computation
		for(k=0; k<N; k++) {
			abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im)); 
		}

		minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
		maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm

		// Find the peak in the PSD from 30 bpm to 180 bpm
		m = minIdx;
		for(k=minIdx; k<(maxIdx); k++) {
			if( abs[k] > abs[m] )
				m = k;
		}

		// Print the heart beat in bpm
		printf( "\n\n\n%d bpm\n\n\n", (m)*60*50/2048 );
	}
	
	close(heartmonfile);
	exit(EXIT_SUCCESS);
}
