/*
 * Copyright (c) 2008, 2009, 2010 Joseph Gaeddert
 * Copyright (c) 2008, 2009, 2010 Virginia Polytechnic
 *                                Institute & State University
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// fixed|float type operations
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquidfpm.internal.h"

#define DEBUG_QFLOAT_ADD    0

#define Q(name)     LIQUIDFPM_CONCAT(q32,name)

float Q(f_fixed_to_float)(Q(f_t) _x)
{
    return powf(2.0f, _x.base) * Q(_fixed_to_float)(_x.frac);
}

Q(f_t) Q(f_float_to_fixed)(float _x)
{
    Q(f_t) xqf;

    // check for zero condition
    if (_x == 0.0f) {
        xqf.base = 0;
        xqf.frac = 0;
        return xqf;
    }

    // base = 2^floor(log2(|x|))
    xqf.base = (int)(floorf(log2f(fabsf(_x))));

    // frac = x / 2^base
    xqf.frac = Q(_float_to_fixed)(_x / (powf(2.0f,xqf.base)));

    return xqf;
}

Q(f_t) Q(f_add)(Q(f_t) _x, Q(f_t) _y)
{
    Q(f_t) sum;
    Q(f_t) max;
    Q(f_t) min;

    // find maximum of _x, _y (ignoring fractional
    // component)
    if (_x.base > _y.base) {
        max.base = _x.base;
        max.frac = _x.frac;

        min.base = _y.base;
        min.frac = _y.frac;
    } else {
        max.base = _y.base;
        max.frac = _y.frac;

        min.base = _x.base;
        min.frac = _x.frac;
    }
    sum.base = max.base;

#if DEBUG_QFLOAT_ADD
    printf(" max = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(max),
                                                max.base,
                                                q32_fixed_to_float(max.frac));
    printf(" min = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(min),
                                                min.base,
                                                q32_fixed_to_float(min.frac));
    printf("\n");

    printf("aligning bases...\n");
#endif
    // align exponents of arguments
    while (min.base < max.base) {
        min.base++;
        min.frac >>= 1;
    }
#if DEBUG_QFLOAT_ADD
    printf(" max = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(max),
                                                max.base,
                                                q32_fixed_to_float(max.frac));
    printf(" min = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(min),
                                                min.base,
                                                q32_fixed_to_float(min.frac));
    printf("\n");

    printf("executing sum...\n");
#endif
    // add fractional components
    sum.frac = min.frac + max.frac;
    //sum.frac = signs_match ? min.frac + min.base :
    //                         min.frac - min.base;
#if DEBUG_QFLOAT_ADD
    printf(" sum = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(sum),
                                                sum.base,
                                                q32_fixed_to_float(sum.frac));
    printf("\n");

    // constrain
    printf("constraining frac...\n");
#endif
    Q(f_constrain)(&sum);
#if DEBUG_QFLOAT_ADD
    printf(" sum = %12.8f = 2^(%4d) * %12.8f\n",Q(f_fixed_to_float)(sum),
                                                sum.base,
                                                q32_fixed_to_float(sum.frac));
    printf("\n");
#endif

    return sum;
}

Q(f_t) Q(f_sub)(Q(f_t) _x, Q(f_t) _y)
{
    _y.frac = -_y.frac;
    return Q(f_add)(_x,_y);
}

Q(f_t) Q(f_mul)(Q(f_t) _x, Q(f_t) _y)
{
    Q(f_t) prod;
    prod.base = _x.base + _y.base;
    prod.frac = Q(_mul)(_x.frac,_y.frac);

    Q(f_constrain)(&prod);

    return prod;
}

Q(f_t) Q(f_div)(Q(f_t) _x, Q(f_t) _y)
{
    unsigned int _n=16;
    Q(f_t) quot;
    quot.base = _x.base - _y.base;
    quot.frac = Q(_div_inv_newton)(_x.frac,_y.frac,_n);

    Q(f_constrain)(&quot);

    return quot;
}

// constrain fractional portion: 1 <= _x.frac < 2
void Q(f_constrain)(Q(f_t) * _x)
{
    // check for zero condidtion
    if (_x->frac == 0) {
        _x->base = 0;
        return;
    }

    // ensure _x.frac < 2
    // TODO : use msb_index for faster implementation
    while (Q(_abs)(_x->frac) >= (Q(_one)<<1)) {
        _x->frac >>= 1;
        _x->base++;
    }

    // ensure _x.frac >= 1
    // TODO : use msb_index for faster implementation
    while (Q(_abs)(_x->frac) < Q(_one)) {
        _x->frac <<= 1;
        _x->base--;
    }
}

