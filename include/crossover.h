/*
 * crossover.h
 *
 *  Created on: 10 de mar de 2020
 *      Author: kulie
 */

#ifndef CROSSOVER_H_
#define CROSSOVER_H_

#include <arm_math.h>

///* These floating point values are used by the filter code below */
//float32_t Fs = 44100;      /* sample rate in samples per second */
//float32_t Pi = 3.141592;   /* the value of Pi */

//typedef struct {
//
///* These floating point values implement the specific filter type */
//float32_t f0 = 100;                /* cut-off (or center) frequency in Hz */
//float32_t Q = 1.5;                 /* filter Q */
//float32_t w0 = 2 * Pi * f0 / Fs;
//float32_t alpha = sin(w0) / (2 * Q);
//float32_t a0 = 1 + alpha;
//float32_t a1 = -2 * cos(w0);
//float32_t a2 = 1 - alpha;
//float32_t b0 = (1 + cos(w0)) / 2;
//float32_t b1 = -(1 + cos(w0));
//float32_t b2 = (1 + cos(w0)) / 2;
//
//} filter_instance_crossover;

/* The Buffer[] array holds the incoming samples, PrevSample[] holds intermediate results */
//float Buffer[1024];         /* this array holds 1024 elements numbered 0 through 1023 */
//float PrevSample[3];        /* this array holds 3 elements numbered 0 through 2 */



void crossover_init(void);
void crossover(float32_t * pSrc, float32_t * pDst, uint16_t blockSize);


#endif /* CROSSOVER_H_ */
