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

#define M_FFT_DECLARE
#include "mgpcl/FFT.h"
#include "mgpcl/Math.h"
#include "mgpcl/SSE.h"

_PS_CONST(4, 4.0f);
static const ALIGN16_BEG float _ps_0123[4] ALIGN16_END = { 0.0f, 1.0f, 2.0f, 3.0f };

#ifdef MGPCL_WIN
#define M_FORCE_INLINE(rtype) rtype __forceinline
#else
#define M_FORCE_INLINE(rtype) __attribute__((always_inline)) rtype
#endif

namespace m
{
    M_FORCE_INLINE(static void) applyFFT_4(const float *aIn, float *aOut, float *bOut, uint32_t stride)
    {
        //applyFFT_ps(aIn, aOut, bOut, 2, 2 * stride)
        float a0 = aIn[0];
        float a1 = aIn[stride << 1];

        aOut[0] = a0 + a1;
        aOut[1] = a0 - a1;
        bOut[0] = 0.0f;
        bOut[1] = 0.0f;

        //applyFFT_ps(aIn + stride, aOut + 2, bOut + 2, 2, 2 * stride)
        a0 = aIn[stride];
        a1 = aIn[3 * stride];

        aOut[2] = a0 + a1;
        aOut[3] = a0 - a1;
        bOut[2] = 0.0f;
        bOut[3] = 0.0f;

        //Here, half = 2, so:
        //    * for i = 0, twA = aOut[2], twB = bOut[2]
        //    * for i = 1, twAng = -2 * PI / 4 = -PI / 2    =>    twA = bOut[3], twB = -aOut[3]

        //i = 0
        float twA = aOut[2];
        float twB = bOut[2];
        float tmp = aOut[0];

        aOut[0] += twA;
        aOut[2] = tmp - twA;

        tmp = bOut[0];
        bOut[0] += twB;
        bOut[2] = tmp - twB;

        //i = 1
        twA = bOut[3];
        twB = -aOut[3];
        tmp = aOut[1];

        aOut[1] += twA;
        aOut[3] = tmp - twA;

        tmp = bOut[1];
        bOut[1] += twB;
        bOut[3] = tmp - twB;
    }

    static void applyFFT_ps(const float *aIn, float *aOut, float *bOut, uint32_t cnt, uint32_t stride)
    {
        if(cnt == 4)
            applyFFT_4(aIn, aOut, bOut, stride);
        else {
            uint32_t half = cnt >> 1;
            uint32_t dbl = stride << 1;
            applyFFT_ps(aIn, aOut, bOut, half, dbl); //TODO: Recursion = bad
            applyFFT_ps(aIn + stride, aOut + half, bOut + half, half, dbl);

            M128 xmm0, xmm7;
            xmm7 = _mm_set1_ps(-2.0f * static_cast<float>(M_PI) / static_cast<float>(cnt));
            xmm0 = _mm_mul_ps(xmm7, _PS_CONST_GET(0123)); //xmm0 = twAng
            xmm7 = _mm_mul_ps(xmm7, _PS_CONST_GET(4));    //xmm7 = twMul

            for(uint32_t i = 0; i < half; i += 4) {
                M128 xmm1, xmm2;
                uint32_t j = half + i;

                sse::sincos_ps(xmm0, &xmm2, &xmm1);       //xmm1 = twCos, xmm2 = twSin

                M128 xmm3, xmm4, xmm5, xmm6;
                xmm3 = _mm_load_ps(aOut + j);    //xmm3 = aOut[j]
                xmm4 = _mm_load_ps(bOut + j);    //xmm4 = bOut[j]

                xmm5 = _mm_mul_ps(xmm1, xmm3);   //xmm5 = twCos * aOut[j]
                xmm6 = _mm_mul_ps(xmm2, xmm4);   //xmm6 = twSin * bOut[j]
                xmm5 = _mm_sub_ps(xmm5, xmm6);   //xmm5 = xmm5 - xmm6 (twA)

                xmm1 = _mm_mul_ps(xmm1, xmm4);   //xmm1 = twCos * bOut[j]
                xmm2 = _mm_mul_ps(xmm2, xmm3);   //xmm2 = twSin * aOut[j]
                xmm6 = _mm_add_ps(xmm1, xmm2);   //xmm6 = xmm1 + xmm2 (twB)

                xmm1 = _mm_load_ps(aOut + i);    //xmm1 = aOut[i] (tmp)
                xmm2 = _mm_add_ps(xmm1, xmm5);   //xmm2 = tmp + twA
                _mm_store_ps(aOut + i, xmm2);    //aOut[i] = xmm2
                xmm2 = _mm_sub_ps(xmm1, xmm5);   //xmm2 = tmp - twA
                _mm_store_ps(aOut + j, xmm2);    //aOut[j] = xmm2

                xmm1 = _mm_load_ps(bOut + i);    //xmm1 = bOut[i] (tmp)
                xmm2 = _mm_add_ps(xmm1, xmm6);   //xmm2 = tmp + twB
                _mm_store_ps(bOut + i, xmm2);    //bOut[i] = xmm2
                xmm2 = _mm_sub_ps(xmm1, xmm6);   //xmm2 = tmp - twB
                _mm_store_ps(bOut + j, xmm2);    //bOut[j] = xmm2

                xmm0 = _mm_add_ps(xmm0, xmm7);   //xmm0 = twAng + twMul
            }
        }
    }

    static void applyFFT_basic(const float *aIn, float *aOut, float *bOut, uint32_t cnt, uint32_t stride)
    {
        if(cnt == 4)
            applyFFT_4(aIn, aOut, bOut, stride);
        else {
            uint32_t half = cnt >> 1;
            uint32_t dbl = stride << 1;
            applyFFT_basic(aIn, aOut, bOut, half, dbl); //TODO: Recursion = bad
            applyFFT_basic(aIn + stride, aOut + half, bOut + half, half, dbl);

            float twMul = -2.0f * static_cast<float>(M_PI) / static_cast<float>(cnt);
            float twAng = 0.0f;

            for(uint32_t i = 0; i < half; i++) {
                uint32_t j = half + i;
                float twCos = Math::cos<float>(twAng);
                float twSin = Math::sin<float>(twAng);
                float twA = twCos * aOut[j] - twSin * bOut[j];
                float twB = twCos * bOut[j] + twSin * aOut[j];

                float tmp = aOut[i];
                aOut[i] += twA;
                aOut[j] = tmp - twA;

                tmp = bOut[i];
                bOut[i] += twB;
                bOut[j] = tmp - twB;

                twAng += twMul;
            }
        }
    }
}

void m::fft::applySSE(const float *aIn, float *aOut, float *bOut, uint32_t cnt)
{
    applyFFT_ps(aIn, aOut, bOut, cnt, 1);
}

void m::fft::apply(const float *aIn, float *aOut, float *bOut, uint32_t cnt)
{
    applyFFT_basic(aIn, aOut, bOut, cnt, 1);
}

void m::fft::absSSE(const float *aIn, const float *bIn, float *result, uint32_t cnt)
{
    M128 xmm0, xmm1;

    for(uint32_t i = 0; i < cnt; i += 4) {
        xmm0 = _mm_load_ps(aIn + i);
        xmm0 = _mm_mul_ps(xmm0, xmm0);

        xmm1 = _mm_load_ps(bIn + i);
        xmm1 = _mm_mul_ps(xmm1, xmm1);

        xmm0 = _mm_add_ps(xmm0, xmm1);
        xmm0 = _mm_sqrt_ps(xmm0);
        _mm_store_ps(result + i, xmm0);
    }
}

void m::fft::abs2SSE(const float *aIn, const float *bIn, float *result, uint32_t cnt, float constant)
{
    M128 xmm0, xmm1, xmm2;
    xmm2 = _mm_set1_ps(constant);

    for(uint32_t i = 0; i < cnt; i += 4) {
        xmm0 = _mm_load_ps(aIn + i);
        xmm0 = _mm_mul_ps(xmm0, xmm0);

        xmm1 = _mm_load_ps(bIn + i);
        xmm1 = _mm_mul_ps(xmm1, xmm1);

        xmm0 = _mm_add_ps(xmm0, xmm1);
        xmm0 = _mm_sqrt_ps(xmm0);
        xmm0 = _mm_mul_ps(xmm0, xmm2);
        _mm_store_ps(result + i, xmm0);
    }
}
