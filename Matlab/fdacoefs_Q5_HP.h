/*
 * Filter Coefficients (C Source) generated by the Filter Design and Analysis Tool
 * Generated by MATLAB(R) 9.2 and the Signal Processing Toolbox 7.4.
 * Generated on: 12-Mar-2020 23:04:23
 */

/*
 * Discrete-Time FIR Filter (real)
 * -------------------------------
 * Filter Structure  : Direct-Form FIR
 * Filter Length     : 300
 * Stable            : Yes
 * Linear Phase      : Yes (Type 4)
 */

/* General type conversion for MATLAB generated C-code  */
#include <arm_math.h>
/* 
 * Expected path to tmwtypes.h 
 * C:\Program Files\MATLAB\R2017a\extern\include\tmwtypes.h 
 */
/*
 * Warning - Filter coefficients were truncated to fit specified data type.  
 *   The resulting response may not match generated theoretical response.
 *   Use the Filter Design & Analysis Tool to design accurate
 *   int16 filter coefficients.
 */
const int NUM_TAPS_Q15 = 300;
const int16_T firCoeffsQ15_H[300] = {
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,     -1,     -1,
       -1,     -1,     -2,     -2,     -2,     -3,     -3,     -3,     -4,
       -4,     -5,     -5,     -6,     -6,     -7,     -8,     -8,     -9,
      -10,    -10,    -11,    -11,    -12,    -12,    -13,    -13,    -14,
      -14,    -14,    -14,    -14,    -14,    -13,    -13,    -12,    -11,
      -10,     -9,     -7,     -6,     -4,     -1,      1,      4,      7,
       11,     14,     18,     22,     27,     32,     37,     42,     47,
       53,     59,     65,     71,     77,     84,     90,     96,    103,
      109,    115,    121,    126,    131,    136,    141,    144,    148,
      150,    152,    153,    153,    153,    151,    148,    144,    138,
      131,    123,    113,    101,     87,     72,     54,     34,     12,
      -13,    -40,    -71,   -104,   -142,   -183,   -228,   -279,   -335,
     -398,   -469,   -549,   -641,   -748,   -873,  -1024,  -1210,  -1447,
    -1762,  -2207,  -2894,  -4110,  -6916, -20848,  20848,   6916,   4110,
     2894,   2207,   1762,   1447,   1210,   1024,    873,    748,    641,
      549,    469,    398,    335,    279,    228,    183,    142,    104,
       71,     40,     13,    -12,    -34,    -54,    -72,    -87,   -101,
     -113,   -123,   -131,   -138,   -144,   -148,   -151,   -153,   -153,
     -153,   -152,   -150,   -148,   -144,   -141,   -136,   -131,   -126,
     -121,   -115,   -109,   -103,    -96,    -90,    -84,    -77,    -71,
      -65,    -59,    -53,    -47,    -42,    -37,    -32,    -27,    -22,
      -18,    -14,    -11,     -7,     -4,     -1,      1,      4,      6,
        7,      9,     10,     11,     12,     13,     13,     14,     14,
       14,     14,     14,     14,     13,     13,     12,     12,     11,
       11,     10,     10,      9,      8,      8,      7,      6,      6,
        5,      5,      4,      4,      3,      3,      3,      2,      2,
        2,      1,      1,      1,      1,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0
};