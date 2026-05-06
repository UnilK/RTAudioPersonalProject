/*
 * Math and dsp utility functions
 * Copyright (C) 2024 Unto Karila
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
*/

#pragma once

#include <complex>

#if defined(_WIN32)

#include <intrin.h>

#else

#if defined(__SSE__)

#include <immintrin.h>

#elif (defined(__ARM_NEON))

#include <arm_neon.h>

#endif

#endif

namespace dsp {

#if (defined(__SSE__) || defined(_M_AMD64) || defined(_M_X64))

struct wtprecalc {
    
    wtprecalc() = default;
    wtprecalc(float frequency, float width, float shift, float gain, float framerate);

    __m128 i_wrot4r, i_wrot4i, i_frot4r, i_frot4i;
    __m128 i_wrot8r, i_wrot8i, i_frot8r, i_frot8i;
    __m128 i_wflr, i_wfli, i_wwlr, i_wwli;
    float i_radius, i_sfreq, i_wfreq, i_normalize;

    __m128 o_wrot4r, o_wrot4i, o_frot4r, o_frot4i;
    __m128 o_wrot8r, o_wrot8i, o_frot8r, o_frot8i;
    __m128 o_wflr, o_wfli, o_wwlr, o_wwli;
    float o_radius, o_sfreq, o_wfreq;
};

#elif defined(__ARM_NEON)

struct wtprecalc {
    
    wtprecalc() = default;
    wtprecalc(float frequency, float width, float shift, float gain, float framerate);

    float32x4_t i_wrot4r, i_wrot4i, i_frot4r, i_frot4i;
    float32x4_t i_wrot8r, i_wrot8i, i_frot8r, i_frot8i;
    float32x4_t i_wflr, i_wfli, i_wwlr, i_wwli;
    float i_radius, i_sfreq, i_wfreq, i_normalize;

    float32x4_t o_wrot4r, o_wrot4i, o_frot4r, o_frot4i;
    float32x4_t o_wrot8r, o_wrot8i, o_frot8r, o_frot8i;
    float32x4_t o_wflr, o_wfli, o_wwlr, o_wwli;
    float o_radius, o_sfreq, o_wfreq;
};

#else

struct wtprecalc {
    wtprecalc() = default;
    wtprecalc(float frequency, float width, float shift, float gain, float framerate);
    float i_radius, i_sfreq, i_wfreq, i_normalize;
    float o_radius, o_sfreq, o_wfreq;
    std::complex<float> i_frot1, i_wrot1, i_frot2, i_wrot2;
    std::complex<float> o_frot1, o_wrot1, o_frot2, o_wrot2;
};

#endif

std::complex<float> read_wavelet(const float *x, float pos, const wtprecalc &p);

void write_wavelet(float *y, float pos, std::complex<float> correlation, const wtprecalc &p);

}
