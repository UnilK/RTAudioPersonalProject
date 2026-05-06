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

#include "waveletio.h"

#include "../math/constants.h"

#include <cmath>

namespace dsp {

#if (defined(__SSE__) || defined(_M_AMD64) || defined(_M_X64))

// x86 SSE ////////////////////////////////////////////////////////////////////////////////////////

union __m128union {
    __m128 v;
    float a[4];
};

static float extract(const __m128 &v, const int &i){
    __m128union u{v};
    return u.a[i];
}

static __m128 fmadd(const __m128 &a, const __m128 &b, const __m128 &c){
    return _mm_add_ps(_mm_mul_ps(a, b), c);
}

static __m128 fmsub(const __m128 &a, const __m128 &b, const __m128 &c){
    return _mm_sub_ps(_mm_mul_ps(a, b), c);
}

static void cmul(
        const __m128 &Xr, const __m128 &Xi,
        const __m128 &Yr, const __m128 &Yi,
        __m128 &Zr, __m128 &Zi)
{
    Zr = fmsub(Xr, Yr, _mm_mul_ps(Xi, Yi));
    Zi = fmadd(Xr, Yi, _mm_mul_ps(Xi, Yr));
}

static void cmule(__m128 &Xr, __m128 &Xi, const __m128 &Yr, const __m128 &Yi){
    
    __m128 tmp0, tmp1;
    cmul(Xr, Xi, Yr, Yi, tmp0, tmp1);
    Xr = tmp0;
    Xi = tmp1;
}

wtprecalc::wtprecalc(float frequency, float width, float shift, float gain, float framerate){
    
    i_radius = (framerate / width) / 2;
    i_sfreq = 2 * PIF * frequency / framerate;
    i_wfreq = PIF / i_radius;
    auto wfi = std::polar(1.0f, -i_sfreq);
    auto wwi = std::polar(1.0f, i_wfreq);

    i_normalize = gain / i_radius;

    o_radius = i_radius / shift;
    o_sfreq = i_sfreq * shift;
    o_wfreq = i_wfreq * shift;
    auto wfo = std::polar(1.0f, o_sfreq);
    auto wwo = std::polar(1.0f, o_wfreq);
    

    {
        auto ww1 = wwi;
        auto ww2 = ww1 * ww1;
        auto ww3 = ww2 * ww1;
        auto ww4 = ww2 * ww2;
        auto ww8 = ww4 * ww4;
        
        auto wf1 = wfi;
        auto wf2 = wf1 * wf1;
        auto wf3 = wf2 * wf1;
        auto wf4 = wf2 * wf2;
        auto wf8 = wf4 * wf4;
        
        i_wrot4r = _mm_set1_ps(ww4.real());
        i_wrot4i = _mm_set1_ps(ww4.imag());
        i_frot4r = _mm_set1_ps(wf4.real());
        i_frot4i = _mm_set1_ps(wf4.imag());
        
        i_wrot8r = _mm_set1_ps(ww8.real());
        i_wrot8i = _mm_set1_ps(ww8.imag());
        i_frot8r = _mm_set1_ps(wf8.real());
        i_frot8i = _mm_set1_ps(wf8.imag());
        
        i_wwlr = _mm_set_ps(ww3.real(), ww2.real(), ww1.real(), 1.0f);
        i_wwli = _mm_set_ps(ww3.imag(), ww2.imag(), ww1.imag(), 0.0f);

        i_wflr = _mm_set_ps(wf3.real(), wf2.real(), wf1.real(), 1.0f);
        i_wfli = _mm_set_ps(wf3.imag(), wf2.imag(), wf1.imag(), 0.0f);
    }

    {
        auto ww1 = wwo;
        auto ww2 = ww1 * ww1;
        auto ww3 = ww2 * ww1;
        auto ww4 = ww2 * ww2;
        auto ww8 = ww4 * ww4;
        
        auto wf1 = wfo;
        auto wf2 = wf1 * wf1;
        auto wf3 = wf2 * wf1;
        auto wf4 = wf2 * wf2;
        auto wf8 = wf4 * wf4;
        
        o_wrot4r = _mm_set1_ps(ww4.real());
        o_wrot4i = _mm_set1_ps(ww4.imag());
        o_frot4r = _mm_set1_ps(wf4.real());
        o_frot4i = _mm_set1_ps(wf4.imag());
        
        o_wrot8r = _mm_set1_ps(ww8.real());
        o_wrot8i = _mm_set1_ps(ww8.imag());
        o_frot8r = _mm_set1_ps(wf8.real());
        o_frot8i = _mm_set1_ps(wf8.imag());
        
        o_wwlr = _mm_set_ps(ww3.real(), ww2.real(), ww1.real(), 1.0f);
        o_wwli = _mm_set_ps(ww3.imag(), ww2.imag(), ww1.imag(), 0.0f);

        o_wflr = _mm_set_ps(wf3.real(), wf2.real(), wf1.real(), 1.0f);
        o_wfli = _mm_set_ps(wf3.imag(), wf2.imag(), wf1.imag(), 0.0f);
    }
}





std::complex<float> read_wavelet(const float *x, float pos, const wtprecalc &p){

    __m128 cr[2] = {0};
    __m128 ci[2] = {0};

    int input_left = std::ceil(pos - p.i_radius);
    int input_right = std::floor(pos + p.i_radius);

    auto wf0 = std::polar<float>(1, -p.i_sfreq * (input_left - pos));
    auto ww0 = std::polar<float>(1, p.i_wfreq * (input_left - pos));
    
    __m128 wwr[2];
    __m128 wwi[2];

    wwr[0] = _mm_set1_ps(ww0.real());
    wwi[0] = _mm_set1_ps(ww0.imag());
    cmule(wwr[0], wwi[0], p.i_wwlr, p.i_wwli);
    cmul(wwr[0], wwi[0], p.i_wrot4r, p.i_wrot4i, wwr[1], wwi[1]);
    
    __m128 wfr = _mm_set1_ps(wf0.real());
    __m128 wfi = _mm_set1_ps(wf0.imag());
    cmule(wfr, wfi, p.i_wflr, p.i_wfli);

    int i = input_left;
    for(; i+7<=input_right; i+=8){
        
        __m128 x0 = _mm_loadu_ps(&x[i]);
        __m128 x1 = _mm_loadu_ps(&x[i+4]);
       
        __m128 wx0 = fmadd(x0, wwr[0], x0);
        cr[0] = fmadd(wfr, wx0, cr[0]);
        ci[0] = fmadd(wfi, wx0, ci[0]);
        
        __m128 wx1 = fmadd(x1, wwr[1], x1);
        cr[1] = fmadd(wfr, wx1, cr[1]);
        ci[1] = fmadd(wfi, wx1, ci[1]);

        cmule(wfr, wfi, p.i_frot8r, p.i_frot8i);
        cmule(wwr[0], wwi[0], p.i_wrot8r, p.i_wrot8i);
        cmule(wwr[1], wwi[1], p.i_wrot8r, p.i_wrot8i);
    }

    __m128 xx[2] = {0};
    if(i+3 <= input_right){
        xx[0] = _mm_loadu_ps(&x[i]);
        i += 4;
        float tmp[4] = {0};
        for(int j=0; i+j<=input_right; j++) tmp[j] = x[i+j];
        xx[1] = _mm_set_ps(tmp[3], tmp[2], tmp[1], tmp[0]);
    } else {
        float tmp[4] = {0};
        for(int j=0; i+j<=input_right; j++) tmp[j] = x[i+j];
        xx[0] = _mm_set_ps(tmp[3], tmp[2], tmp[1], tmp[0]);
    }
    
    __m128 wx0 = fmadd(xx[0], wwr[0], xx[0]);
    cr[0] = fmadd(wfr, wx0, cr[0]);
    ci[0] = fmadd(wfi, wx0, ci[0]);
    
    __m128 wx1 = fmadd(xx[1], wwr[1], xx[1]);
    cr[1] = fmadd(wfr, wx1, cr[1]);
    ci[1] = fmadd(wfi, wx1, ci[1]);

    cmule(cr[1], ci[1], p.i_frot4r, p.i_frot4i);

    cr[0] = _mm_add_ps(cr[0], cr[1]);
    ci[0] = _mm_add_ps(ci[0], ci[1]);
    cr[0] = _mm_add_ps(cr[0], _mm_shuffle_ps(cr[0], cr[0], 0b00001011));
    ci[0] = _mm_add_ps(ci[0], _mm_shuffle_ps(ci[0], ci[0], 0b00001011));
    cr[0] = _mm_add_ps(cr[0], _mm_shuffle_ps(cr[0], cr[0], 0b00000001));
    ci[0] = _mm_add_ps(ci[0], _mm_shuffle_ps(ci[0], ci[0], 0b00000001));

    return std::complex<float>(_mm_cvtss_f32(cr[0]), _mm_cvtss_f32(ci[0])) * p.i_normalize;
}





void write_wavelet(float *y, float pos, std::complex<float> correlation, const wtprecalc &p){

    int output_left = std::ceil(pos - p.o_radius);
    int output_right = std::floor(pos + p.o_radius);

    auto wf0 = std::polar<float>(1, p.o_sfreq * (output_left - pos)) * correlation;
    auto ww0 = std::polar<float>(1, p.o_wfreq * (output_left - pos));
    
    __m128 wwr[2], wfr[2];
    __m128 wwi[2], wfi[2];

    wwr[0] = _mm_set1_ps(ww0.real());
    wwi[0] = _mm_set1_ps(ww0.imag());
    cmule(wwr[0], wwi[0], p.o_wwlr, p.o_wwli);
    cmul(wwr[0], wwi[0], p.o_wrot4r, p.o_wrot4i, wwr[1], wwi[1]);
    
    wfr[0] = _mm_set1_ps(wf0.real());
    wfi[0] = _mm_set1_ps(wf0.imag());
    cmule(wfr[0], wfi[0], p.o_wflr, p.o_wfli);
    cmul(wfr[0], wfi[0], p.o_frot4r, p.o_frot4i, wfr[1], wfi[1]);

    int i=output_left;
    for(; i+7<=output_right; i+=8){

        __m128 y0 = _mm_loadu_ps(&y[i]);
        __m128 y1 = _mm_loadu_ps(&y[i+4]);
       
        y0 = _mm_add_ps(y0, fmadd(wfr[0], wwr[0], wfr[0]));
        y1 = _mm_add_ps(y1, fmadd(wfr[1], wwr[1], wfr[1]));
        
        _mm_storeu_ps(&y[i], y0);
        _mm_storeu_ps(&y[i+4], y1);

        cmule(wfr[0], wfi[0], p.o_frot8r, p.o_frot8i);
        cmule(wfr[1], wfi[1], p.o_frot8r, p.o_frot8i);
        cmule(wwr[0], wwi[0], p.o_wrot8r, p.o_wrot8i);
        cmule(wwr[1], wwi[1], p.o_wrot8r, p.o_wrot8i);
    }
   
    __m128 wf, ww;

    if(i+3 <= output_right){
        
        __m128 y0 = _mm_loadu_ps(&y[i]);
        y0 = _mm_add_ps(y0, fmadd(wfr[0], wwr[0], wfr[0]));
        _mm_storeu_ps(&y[i], y0);
        
        wf = wfr[1];
        ww = wwr[1];
        
        i += 4;
    
    } else {    
        wf = wfr[0];
        ww = wwr[0];
    }

    for(int j=0; i+j <= output_right; j++) y[i+j] += (1.0f + extract(ww, j)) * extract(wf, j);
}

#elif defined(__ARM_NEON)

// ARM NEON ///////////////////////////////////////////////////////////////////////////////////////

static float32x4_t fmadd(const float32x4_t &a, const float32x4_t &b, const float32x4_t &c){
    return vfmaq_f32(c, a, b);
}

static float32x4_t fmsub(const float32x4_t &a, const float32x4_t &b, const float32x4_t &c){
    return vfmsq_f32(c, a, b);
}

static float32x4_t set1(float val){
    return {val, val, val, val};
}

static float32x4_t set(float x3, float x2, float x1, float x0){
    return {x0, x1, x2, x3};
}

static float32x4_t loadu(const float *x){
    return vld1q_f32(x);
}

static void storeu(float *y, float32x4_t val){
    vst1q_f32(y, val);
}

static void cmul(
        const float32x4_t &Xr, const float32x4_t &Xi,
        const float32x4_t &Yr, const float32x4_t &Yi,
        float32x4_t &Zr, float32x4_t &Zi)
{
    Zr = fmsub(Xi, Yi, vmulq_f32(Xr, Yr));
    Zi = fmadd(Xr, Yi, vmulq_f32(Xi, Yr));
}

static void cmule(float32x4_t &Xr, float32x4_t &Xi, const float32x4_t &Yr, const float32x4_t &Yi){
    
    float32x4_t tmp0, tmp1;
    cmul(Xr, Xi, Yr, Yi, tmp0, tmp1);
    Xr = tmp0;
    Xi = tmp1;
}

wtprecalc::wtprecalc(float frequency, float width, float shift, float gain, float framerate){
    
    i_radius = (framerate / width) / 2;
    i_sfreq = 2 * PIF * frequency / framerate;
    i_wfreq = PIF / i_radius;
    auto wfi = std::polar(1.0f, -i_sfreq);
    auto wwi = std::polar(1.0f, i_wfreq);

    i_normalize = gain / i_radius;

    o_radius = i_radius / shift;
    o_sfreq = i_sfreq * shift;
    o_wfreq = i_wfreq * shift;
    auto wfo = std::polar(1.0f, o_sfreq);
    auto wwo = std::polar(1.0f, o_wfreq);
    

    {
        auto ww1 = wwi;
        auto ww2 = ww1 * ww1;
        auto ww3 = ww2 * ww1;
        auto ww4 = ww2 * ww2;
        auto ww8 = ww4 * ww4;
        
        auto wf1 = wfi;
        auto wf2 = wf1 * wf1;
        auto wf3 = wf2 * wf1;
        auto wf4 = wf2 * wf2;
        auto wf8 = wf4 * wf4;
        
        i_wrot4r = set1(ww4.real());
        i_wrot4i = set1(ww4.imag());
        i_frot4r = set1(wf4.real());
        i_frot4i = set1(wf4.imag());
        
        i_wrot8r = set1(ww8.real());
        i_wrot8i = set1(ww8.imag());
        i_frot8r = set1(wf8.real());
        i_frot8i = set1(wf8.imag());
        
        i_wwlr = set(ww3.real(), ww2.real(), ww1.real(), 1.0f);
        i_wwli = set(ww3.imag(), ww2.imag(), ww1.imag(), 0.0f);

        i_wflr = set(wf3.real(), wf2.real(), wf1.real(), 1.0f);
        i_wfli = set(wf3.imag(), wf2.imag(), wf1.imag(), 0.0f);
    }

    {
        auto ww1 = wwo;
        auto ww2 = ww1 * ww1;
        auto ww3 = ww2 * ww1;
        auto ww4 = ww2 * ww2;
        auto ww8 = ww4 * ww4;
        
        auto wf1 = wfo;
        auto wf2 = wf1 * wf1;
        auto wf3 = wf2 * wf1;
        auto wf4 = wf2 * wf2;
        auto wf8 = wf4 * wf4;
        
        o_wrot4r = set1(ww4.real());
        o_wrot4i = set1(ww4.imag());
        o_frot4r = set1(wf4.real());
        o_frot4i = set1(wf4.imag());
        
        o_wrot8r = set1(ww8.real());
        o_wrot8i = set1(ww8.imag());
        o_frot8r = set1(wf8.real());
        o_frot8i = set1(wf8.imag());
        
        o_wwlr = set(ww3.real(), ww2.real(), ww1.real(), 1.0f);
        o_wwli = set(ww3.imag(), ww2.imag(), ww1.imag(), 0.0f);

        o_wflr = set(wf3.real(), wf2.real(), wf1.real(), 1.0f);
        o_wfli = set(wf3.imag(), wf2.imag(), wf1.imag(), 0.0f);
    }
}





std::complex<float> read_wavelet(const float *x, float pos, const wtprecalc &p){

    float32x4_t cr[2] = {0};
    float32x4_t ci[2] = {0};

    int input_left = std::ceil(pos - p.i_radius);
    int input_right = std::floor(pos + p.i_radius);

    auto wf0 = std::polar<float>(1, -p.i_sfreq * (input_left - pos));
    auto ww0 = std::polar<float>(1, p.i_wfreq * (input_left - pos));
    
    float32x4_t wwr[2];
    float32x4_t wwi[2];

    wwr[0] = set1(ww0.real());
    wwi[0] = set1(ww0.imag());
    cmule(wwr[0], wwi[0], p.i_wwlr, p.i_wwli);
    cmul(wwr[0], wwi[0], p.i_wrot4r, p.i_wrot4i, wwr[1], wwi[1]);
    
    float32x4_t wfr = set1(wf0.real());
    float32x4_t wfi = set1(wf0.imag());
    cmule(wfr, wfi, p.i_wflr, p.i_wfli);

    int i = input_left;
    for(; i+7<=input_right; i+=8){
        
        float32x4_t x0 = loadu(&x[i]);
        float32x4_t x1 = loadu(&x[i+4]);
       
        float32x4_t wx0 = fmadd(x0, wwr[0], x0);
        cr[0] = fmadd(wfr, wx0, cr[0]);
        ci[0] = fmadd(wfi, wx0, ci[0]);
        
        float32x4_t wx1 = fmadd(x1, wwr[1], x1);
        cr[1] = fmadd(wfr, wx1, cr[1]);
        ci[1] = fmadd(wfi, wx1, ci[1]);

        cmule(wfr, wfi, p.i_frot8r, p.i_frot8i);
        cmule(wwr[0], wwi[0], p.i_wrot8r, p.i_wrot8i);
        cmule(wwr[1], wwi[1], p.i_wrot8r, p.i_wrot8i);
    }

    float32x4_t xx[2] = {0};
    if(i+3 <= input_right){
        xx[0] = loadu(&x[i]);
        i += 4;
        for(int j=0; i+j<=input_right; j++) xx[1][j] = x[i+j];
    } else {
        for(int j=0; i+j<=input_right; j++) xx[0][j] = x[i+j];
    }
    
    float32x4_t wx0 = fmadd(xx[0], wwr[0], xx[0]);
    cr[0] = fmadd(wfr, wx0, cr[0]);
    ci[0] = fmadd(wfi, wx0, ci[0]);
    
    float32x4_t wx1 = fmadd(xx[1], wwr[1], xx[1]);
    cr[1] = fmadd(wfr, wx1, cr[1]);
    ci[1] = fmadd(wfi, wx1, ci[1]);

    cmule(cr[1], ci[1], p.i_frot4r, p.i_frot4i);

    cr[0] = vaddq_f32(cr[0], cr[1]);
    ci[0] = vaddq_f32(ci[0], ci[1]);
    cr[0][0] = (cr[0][0] + cr[0][1]) + (cr[0][2] + cr[0][3]);
    ci[0][0] = (ci[0][0] + ci[0][1]) + (ci[0][2] + ci[0][3]);

    return std::complex<float>(cr[0][0], ci[0][0]) * p.i_normalize;
}





void write_wavelet(float *y, float pos, std::complex<float> correlation, const wtprecalc &p){

    int output_left = std::ceil(pos - p.o_radius);
    int output_right = std::floor(pos + p.o_radius);

    auto wf0 = std::polar<float>(1, p.o_sfreq * (output_left - pos)) * correlation;
    auto ww0 = std::polar<float>(1, p.o_wfreq * (output_left - pos));
    
    float32x4_t wwr[2], wfr[2];
    float32x4_t wwi[2], wfi[2];

    wwr[0] = set1(ww0.real());
    wwi[0] = set1(ww0.imag());
    cmule(wwr[0], wwi[0], p.o_wwlr, p.o_wwli);
    cmul(wwr[0], wwi[0], p.o_wrot4r, p.o_wrot4i, wwr[1], wwi[1]);
    
    wfr[0] = set1(wf0.real());
    wfi[0] = set1(wf0.imag());
    cmule(wfr[0], wfi[0], p.o_wflr, p.o_wfli);
    cmul(wfr[0], wfi[0], p.o_frot4r, p.o_frot4i, wfr[1], wfi[1]);

    int i=output_left;
    for(; i+7<=output_right; i+=8){

        float32x4_t y0 = loadu(&y[i]);
        float32x4_t y1 = loadu(&y[i+4]);
       
        y0 = vaddq_f32(y0, fmadd(wfr[0], wwr[0], wfr[0]));
        y1 = vaddq_f32(y1, fmadd(wfr[1], wwr[1], wfr[1]));
        
        storeu(&y[i], y0);
        storeu(&y[i+4], y1);

        cmule(wfr[0], wfi[0], p.o_frot8r, p.o_frot8i);
        cmule(wfr[1], wfi[1], p.o_frot8r, p.o_frot8i);
        cmule(wwr[0], wwi[0], p.o_wrot8r, p.o_wrot8i);
        cmule(wwr[1], wwi[1], p.o_wrot8r, p.o_wrot8i);
    }
   
    float32x4_t wf, ww;

    if(i+3 <= output_right){
        
        float32x4_t y0 = loadu(&y[i]);
        y0 = vaddq_f32(y0, fmadd(wfr[0], wwr[0], wfr[0]));
        storeu(&y[i], y0);
        
        wf = wfr[1];
        ww = wwr[1];
        
        i += 4;
    
    } else {    
        wf = wfr[0];
        ww = wwr[0];
    }

    for(int j=0; i+j <= output_right; j++) y[i+j] += (1.0f + ww[j]) * wf[j];
}

#else

// DEFAULT ////////////////////////////////////////////////////////////////////////////////////////

wtprecalc::wtprecalc(float frequency, float width, float shift, float gain, float framerate){
    
    i_radius = (framerate / width) / 2;
    i_sfreq = 2 * PIF * frequency / framerate;
    i_wfreq = PIF / i_radius;
    i_frot1 = std::polar(1.0f, -i_sfreq);
    i_wrot1 = std::polar(1.0f, i_wfreq);
    i_frot2 = i_frot1 * i_frot1;
    i_wrot2 = i_wrot1 * i_wrot1;

    i_normalize = gain / i_radius;

    o_radius = i_radius / shift;
    o_sfreq = i_sfreq * shift;
    o_wfreq = i_wfreq * shift;
    o_frot1 = std::polar(1.0f, o_sfreq);
    o_wrot1 = std::polar(1.0f, o_wfreq);
    o_frot2 = o_frot1 * o_frot1;
    o_wrot2 = o_wrot1 * o_wrot1;
}

std::complex<float> read_wavelet(const float *x, float pos, const wtprecalc &p){
    
    std::complex<float> correlation[2] = {0.0f, 0.0f};

    int input_left = std::ceil(pos - p.i_radius);
    int input_right = std::floor(pos + p.i_radius);

    auto wf = std::polar<float>(1, -p.i_sfreq * (input_left - pos));
    
    std::complex<float> ww[2];
    ww[0] = std::polar<float>(1, p.i_wfreq * (input_left - pos));
    ww[1] = ww[0] * p.i_wrot1;

    int i = input_left;
    for(; i+1<=input_right; i+=2){
        correlation[0] += x[i] * (1.0f + ww[0].real()) * wf;
        correlation[1] += x[i+1] * (1.0f + ww[1].real()) * wf;
        wf *= p.i_frot2;
        ww[0] *= p.i_wrot2;
        ww[1] *= p.i_wrot2;
    }

    if(i <= input_right){
        correlation[0] += x[i] * (1.0f + ww[0].real()) * wf;
    }
    
    return (correlation[0] + correlation[1] * p.i_frot1) * p.i_normalize;
}

void write_wavelet(float *y, float pos, std::complex<float> correlation, const wtprecalc &p){

    int output_left = std::ceil(pos - p.o_radius);
    int output_right = std::floor(pos + p.o_radius);

    std::complex<float> wf[2];
    std::complex<float> ww[2];
    
    wf[0] = std::polar<float>(1, p.o_sfreq * (output_left - pos)) * correlation;
    ww[0] = std::polar<float>(1, p.o_wfreq * (output_left - pos));
    wf[1] = wf[0] * p.o_frot1;
    ww[1] = ww[0] * p.o_wrot1;

    int i=output_left;
    for(; i+1<=output_right; i+=2){
        y[i] += (1.0f + ww[0].real()) * wf[0].real();
        y[i+1] += (1.0f + ww[1].real()) * wf[1].real();
        wf[0] *= p.o_frot2;
        ww[0] *= p.o_wrot2;
        wf[1] *= p.o_frot2;
        ww[1] *= p.o_wrot2;
    }
    
    if(i <= output_right){
        y[i] += (1.0f + ww[0].real()) * wf[0].real();
    }
}

#endif

}
