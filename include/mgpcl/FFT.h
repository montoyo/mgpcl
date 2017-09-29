/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include <cstdint>

#ifdef M_FFT_DECLARE
#define M_FFT_PREFIX
#else
#define M_FFT_PREFIX extern
#endif

namespace m
{
    namespace fft
    {
        /* Fast Fourier Transform computation functions
         * --------------------------------------------
         * For applySSE and apply, 'cnt' must be a power of two.
         * For every SSE functions, all float arrays must be 16-bytes aligned.
         *
         * For both apply functions, 'aIn' is the array of input samples, 'aOut' is the
         * output array of real parts and 'bOut' is the output array of imaginary parts.
         * 'cnt' is the number of elements in the arrays, which should be the same for
         * 'aIn', 'aOut', and 'bOut'.
         *
         * absSSE computes the modulus [that is, sqrt(a^2 + b^2)] of each complex numbers
         * in 'aIn' and 'bIn', and outputs the result in 'result'. abs2SSE does the same
         * but it also multiplies the result by some constant, which might be useful.
         *
         * Also note that 'cnt' must be divisible by 4 (since SSE treats 4 floats).
         */

        M_FFT_PREFIX void apply(const float *aIn, float *aOut, float *bOut, uint32_t cnt);
        M_FFT_PREFIX void applySSE(const float *aIn, float *aOut, float *bOut, uint32_t cnt);                        //Requires 16-bytes aligned float arrays!!!
        M_FFT_PREFIX void absSSE(const float *aIn, const float *bIn, float *result, uint32_t cnt);                    //Requires 16-bytes aligned float arrays and cnt % 4 == 0
        M_FFT_PREFIX void abs2SSE(const float *aIn, const float *bIn, float *result, uint32_t cnt, float constant);    //Requires 16-bytes aligned float arrays and cnt % 4 == 0
    }
}
