/* -*- C++ -*-
 * 
 *  graphics_sse2.cpp - graphics routines using X86 SSE2 cpu functionality
 *
 *  Copyright (c) 2009 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Based upon routines provided by Roto

#ifdef USE_X86_GFX

#include <SDL.h>
#include <emmintrin.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "graphics_x86_common.h"


void imageFilterMean_SSE2(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
    int i = 0;

    // Compute first few values so we're on a 16-byte boundary in dst
    for (; !is_aligned(dst + i, 16) && (i < length); i++) {
        dst[i] = mean_pixel(src1[i], src2[i]);
    }

    // Do bulk of processing using SSE2 (find the mean of 16 8-bit unsigned integers, with saturation)
    __m128i mask = _mm_set1_epi8(0x7F);
    for (; i < length - 15; i += 16) {
        __m128i s1 = _mm_loadu_si128((__m128i*)(src1 + i));
        s1 = _mm_srli_epi16(s1, 1); // shift right 1
        s1 = _mm_and_si128(s1, mask); // apply byte-mask
        __m128i s2 = _mm_loadu_si128((__m128i*)(src2 + i));
        s2 = _mm_srli_epi16(s2, 1); // shift right 1
        s2 = _mm_and_si128(s2, mask); // apply byte-mask
        __m128i r = _mm_adds_epu8(s1, s2);
        _mm_store_si128((__m128i*)(dst + i), r);
    }

    // If any bytes are left over, deal with them individually
    for (; i < length; i++) {
        dst[i] = mean_pixel(src1[i], src2[i]);
    }
}


void imageFilterAddTo_SSE2(unsigned char *dst, unsigned char *src, int length)
{
    int i = 0;

    // Compute first few values so we're on a 16-byte boundary in dst
    for (; !is_aligned(dst + i, 16) && (i < length); i++) {
        addto_pixel(dst[i], src[i]);
    }

    // Do bulk of processing using SSE2 (add 16 8-bit unsigned integers, with saturation)
    for (; i < length - 15; i += 16) {
        __m128i s = _mm_loadu_si128((__m128i*)(src + i));
        __m128i d = _mm_load_si128((__m128i*)(dst + i));
        __m128i r = _mm_adds_epu8(s, d);
        _mm_store_si128((__m128i*)(dst + i), r);
    }

    // If any bytes are left over, deal with them individually
    for (; i < length; i++) {
        addto_pixel(dst[i], src[i]);
    }
}


void imageFilterSubFrom_SSE2(unsigned char *dst, unsigned char *src, int length)
{
    int i = 0;

    // Compute first few values so we're on a 16-byte boundary in dst
    for (; !is_aligned(dst + i, 16) && (i < length); i++) {
        subfrom_pixel(dst[i], src[i]);
    }

    // Do bulk of processing using SSE2 (sub 16 8-bit unsigned integers, with saturation)
    for (; i < length - 15; i += 16) {
        __m128i s = _mm_loadu_si128((__m128i*)(src + i));
        __m128i d = _mm_load_si128((__m128i*)(dst + i));
        __m128i r = _mm_subs_epu8(d, s);
        _mm_store_si128((__m128i*)(dst + i), r);
    }

    // If any bytes are left over, deal with them individually
    for (; i < length; i++) {
        subfrom_pixel(dst[i], src[i]);
    }
}

void imageFilterBlend_SSE2(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length)
{
    imageFilterBlend_SSE_Common(dst_buffer, src_buffer, alphap, alpha, length);
}

bool alphaMaskBlend_SSE2(SDL_Surface* dst, SDL_Surface *s1, SDL_Surface *s2, SDL_Surface *mask_surface, const SDL_Rect& rect, Uint32 mask_value)
{
    return alphaMaskBlend_SSE_Common(dst, s1, s2, mask_surface, rect, mask_value);
}

void alphaMaskBlendConst_SSE2(SDL_Surface* dst, SDL_Surface *s1, SDL_Surface *s2, const SDL_Rect& rect, Uint32 mask_value)
{
    alphaMaskBlendConst_SSE_Common(dst, s1, s2, rect, mask_value);
}

#endif
