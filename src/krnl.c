/* 
 * Description: 
 * This file contains a (Xilinx Vitis) HLS implementation of the 
 * probability calculation function in the LoFreq variant calling tool. 
 * The function's original C implementation is here: 
 *      https://github.com/CSB5/lofreq/blob/8fe42b/src/lofreq/snpcaller.c#L818.
 * 
 * The krnl function is the top level function (hardware module).
 */

#include "krnl.h"

/* The LogSumExp function used in the probability calculation. */
double log_sum_exp(
        double log_a, 
        double log_b
        )
{
	// The expression is broken down into elementary operations 
	// in order to use pragmas for controlling latency and impl fabric.
	double log_delta = 0.0; 
	double exp_value = 0.0;
	double exp_add1_value = 0.0;
	double log1p_value = 0.0; 
	double final_sum = 0.0; 
#pragma HLS bind_op variable=log_delta op=dsub impl=fabric
#pragma HLS bind_op variable=exp_value op=dexp impl=meddsp latency=20
#pragma HLS bind_op variable=exp_add1_value op=dadd impl=fabric
#pragma HLS bind_op variable=log1p_value op=dlog impl=meddsp latency=20
#pragma HLS bind_op variable=final_sum op=dadd impl=fabric

	if (log_a > log_b) {
		log_delta = log_b - log_a;
		exp_value = exp(log_delta);
		exp_add1_value = 1.0 + exp_value;
		log1p_value = log(exp_add1_value); 
		final_sum = log_a + log1p_value; 
		return final_sum; 

	} else {
		log_delta = log_a - log_b; 
		exp_value = exp(log_delta); 
		exp_add1_value = 1.0 + exp_value; 
		log1p_value = log(exp_add1_value); 
		final_sum = log_b + log1p_value; 
		return final_sum; 
	}
}

/* 
 * Computes the logarithm of error probabilities with special case handling. 
 * The output will be stored using the output pointers.
 * 
 * Input: 
 *      pn: error probability used in iteration n.
 *      output1: pointer, used to store the value of log(pn).
 *      output2: pointer, used to store the value of log(1.0 - pn).
 *
 * Inlining of this function is disabled intentionally.
 * The goal is to force task-level parallelization 
 * (mem_load, log_func, pe_array_compute).
 * */
void log_func(
        double pn, 
        double * output1, 
        double * output2
        ) 
{ 
#pragma HLS INLINE off

	double log_pn = 1.0; 
	double log_1_pn = 1.0;

	if (fabs(pn) < DBL_EPSILON) {
		log_pn = log(DBL_EPSILON); 
	} else {
		log_pn = log(pn);
	}

	if (fabs(pn - 1.0) < DBL_EPSILON) {
		log_1_pn = log(DBL_EPSILON);
	} else {
		log_1_pn = log(1.0 - pn);
	}

	*output1 = log_pn;
	*output2 = log_1_pn;
} 

/* 
 * The prefetching unit. Accesses memory for error probability values.
 *
 * Input: 
 *      err_probs: pointer to the error probability array.
 *      n: index.
 *      pn: pointer, used to store the accessed value.
 *
 * Inlining of this function is disabled intentionally.
 * The goal is to force task-level parallelization
 * (mem_load, log_func, pe_array_compute).
 * */
void mem_load(
        const double * err_probs, 
        int n, 
        double * pn
        ) 
{
#pragma HLS INLINE off
	*pn = err_probs[n];
}

/* 
 * Implements the inner loop in the original probability calculation function.
 * The computed results will be stored in the output buffer.
 * 
 * Input:
 *      log_pn: precomputed log(pn). 
 *      log_1_pn: precomputed log(1-pn).
 *      K: the top function input argument.
 *      input_ptr: pointer to input buffer. 
 *      output_ptr: pointer to the output buffer. 
 * */
void pe_array_internal(
        double log_pn, 
        double log_1_pn, 
        int K, 
        double * input_ptr, 
        double * output_ptr
        ) 
{
#pragma HLS INLINE off 
	int ind = 0;         
 
	double pprev_k = 0.0;
	double pprev_k_1 = 0.0;

	// input to the log_sum_exp function
	double in1 = 0.0;
	double in2 = 0.0;

	block_loop: for (int k = 0; k <= K; k += UNROLL_FACTOR) { 
		thread_loop: for (int kk = 0; kk < UNROLL_FACTOR; kk++) { 
#pragma HLS UNROLL factor=8       
			ind = k + kk;
			pprev_k = input_ptr[ind];
			pprev_k_1 = (0 == ind) ? 0 : input_ptr[ind - 1];

			in1 = (ind == K) ? pprev_k : (pprev_k + log_1_pn); 
			in2 = pprev_k_1 + log_pn;
			output_ptr[ind] = (0 == ind) ? in1 : log_sum_exp(in1, in2); 
		}
	}
}

/* 
 * Double buffering: the input and output buffer switches 
 * every outer loop iteration. The input argument *use_prev* to this function
 * determines which is the input and which is the output.
 *
 * Inlining of this function is disabled intentionally.
 * The goal is to force task-level parallelization 
 * (mem_load, log_func, pe_array_compute).
 * */
void pe_array_compute(
        double log_pn, 
        double log_1_pn, 
        int K, 
        char use_prev, 
        double * probvec_prev, 
        double * probvec
        )
{
#pragma HLS INLINE off
    if (1 == (use_prev & 1)) {
        pe_array_internal(log_pn, log_1_pn, K, probvec_prev, probvec);
    }
    else {
        pe_array_internal(log_pn, log_1_pn, K, probvec, probvec_prev); 
    }
}


/* 
 * The Top-level function.
 * Implements the pruned_calc_prob_dist function in the LoFreq project, 
 * targetting Xilinx Alveo FPGAs.
 * The input is identical to that to the pruned_calc_prob_dist function.
 *
 * Input: 
 *      err_probs: input error probability array.
 *      N: total number of bases of the current column.
 *      K: number of observed varying bases of the current column.
 *
 *      probvec_output: pointer to the output array.
 * */
void krnl(
        const double * err_probs, 
        int N,
        int K,
        double * probvec_output
        ) 
{
	// Both probvec and probvec_prev are used to store computed probability 
	// values. In each outer loop iteration, one is used as input buffer 
	// (to read from), while the other is used as output buffer (to write to). 
	// The outer loop variable *n* tracks this information.
	// 
	// The naming of the two arrays follows the convention of the original
	// function in LoFreq. probvec_prev isn't always the input buffer.
	double probvec[BUFFER_SIZE];
	double probvec_prev[BUFFER_SIZE];
#pragma HLS array_partition variable=probvec cyclic factor=8
#pragma HLS array_partition variable=probvec_prev cyclic factor=8
#pragma HLS bind_storage variable=probvec type=RAM_2P impl=URAM latency=3
#pragma HLS bind_storage variable=probvec_prev type=RAM_2P impl=URAM latency=3 

	int K_add_1 = K + 1;
	int K_minus_1 = K - 1; 
	int n;
	char use_prev = 1;	
	// use_prev: counting outer loop, used for double buffering
	// the use of use_prev instead of n for this purpose is beneficial 
	// to the routing process.

	double pn = 0.0;	// error prob for the n-th outer loop iteration
	double pn_next = 0.0;	// error prob for the (n + 1)-th outer loop iteration 
	double * pn_next_ptr = &pn_next;

	double log_pn = 1.0;	// log(pn) for (outer loop) iteration n
	double * log_pn_ptr = &log_pn;

	double log_1_pn = 1.0;	// log(1.0 - pn) for (outer loop) iteration n
	double * log_1_pn_ptr = &log_1_pn;
	
	double log_pn_next = 1.0; // log(pn) for (outer loop) iteration (n + 1)
	double * log_pn_next_ptr = &log_pn_next;

	double log_1_pn_next = 1.0; // log(1.0 - pn) for (outer loop) iteration (n + 1)
	double * log_1_pn_next_ptr = &log_1_pn_next;
 
	// The outer loop is split into three loops (loop_n_lt_K, loop_n_eq_K,
	// loop_n_gt_K), in order to eliminate control logics.

	// The main outer loop logic consists of a task-level pipeline:
	// stage 1: prefetching (mem_load)
	// stage 2: computing logarithm (log_func)
	// stage 3: probability array calculation (pe_array_compute)
	// 
	// Note that all three functions (mem_load, log_func, pe_array_compute) 
	// must be disabled for inline in order to force parallelization. 
	// By doing so, each function will be implemented in an individual 
	// hardware unit with its separate level of hierarchy in the RTL.

	// prefetching and computing log ahead in order to start the pipeline
	pn = err_probs[1];
	probvec_prev[0] = 0.0;
	log_func(err_probs[0], log_pn_ptr, log_1_pn_ptr); 

	// outer loop iterations where n < K:
	loop_n_lt_K: for (n = 1; n < K; n++) {

		if (1 == (use_prev & 1)) 
			probvec_prev[n] = LOGZERO;
		else
			probvec[n] = LOGZERO;

		mem_load(err_probs, (n + 1), pn_next_ptr);			
		log_func(pn, log_pn_next_ptr, log_1_pn_next_ptr);	
		pe_array_compute(log_pn, log_1_pn, K, use_prev, probvec_prev, probvec);

		log_pn = log_pn_next;
		log_1_pn = log_1_pn_next;
		pn = pn_next;

		use_prev ^= 1;
	}
	
	// special iteration where n == K:
	loop_n_eq_K: {
		pn = err_probs[K_minus_1];
		log_func(pn, log_pn_ptr, log_1_pn_ptr);
		pe_array_compute(log_pn, log_1_pn, K, use_prev, probvec_prev, probvec); 

		// handling special case: n == K  
		if (1 == (use_prev & 1)) 
			probvec[K] = probvec_prev[K_minus_1] + log_pn; 
		else
			probvec_prev[K] = probvec[K_minus_1] + log_pn;
		
		use_prev ^= 1;
	}
	pn = err_probs[K]; 
	log_func(pn, log_pn_ptr, log_1_pn_ptr); 
	pn = err_probs[K_add_1]; 

	// outer loop iterations where n > K:
	loop_n_gt_K: for (n = K_add_1; n <= N; n++) {

		mem_load(err_probs, (n + 1), pn_next_ptr);			
		log_func(pn, log_pn_next_ptr, log_1_pn_next_ptr);	
		pe_array_compute(log_pn, log_1_pn, K, use_prev, probvec_prev, probvec);

		log_pn = log_pn_next;
		log_1_pn = log_1_pn_next;
		pn = pn_next;

		use_prev ^= 1;
	}

	// Writing the output to the output buffer (in DDR)
	normal_exit: for (int j = 0; j < K_add_1; j++) {
		double output = (1 == (use_prev & 1)) ? probvec_prev[j] : probvec[j]; 
		probvec_output[j] = output; 
	} 
	return;
}
