/* Header file, macro definitions for the HLS C code. */

#ifndef KRNL_H
#define KRNL_H

#include <math.h>
#include <float.h>

#define BUFFER_SIZE 65536 // intermediate buffer size
#define UNROLL_FACTOR 8 // number of PEs parallelizing the inner loop
#define LOGZERO -1e100

void krnl(const double * err_probs, int N, int K, double * probvec_output);
#endif
