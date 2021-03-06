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
// gentab.sincos.pwpoly.c
//
// Piece-wise polynomial fit to first quadrant of sin(x). Each
// element in the table is a second-order polynomial.
//
// See also:
//   scripts/piecewise_poly_fit.m
//

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "liquidfpm.internal.h"

void usage(void)
{
    printf("gentab.sincos.pwpolyfit usage:\n");
    printf("  u/h   :   print this help file\n");
    printf("    n   :   name (e.g. q32b16)\n");
    printf("    i   :   intbits (including sign bit)\n");
    printf("    f   :   fracbits\n");
    //printf("    o   :   output filename [default: standard output]\n");
}


int main(int argc, char * argv[]) {
    // options
    FILE * fid = stdout;
    char qtype[64] = "q32";
    unsigned int intbits = 7;
    unsigned int fracbits = 25;

    // read options
    int dopt;
    while ((dopt = getopt(argc,argv,"uhn:i:f:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();                    return 0;
        case 'n':   strncpy(qtype,optarg,64);   break;
        case 'i':   intbits = atoi(optarg);     break;
        case 'f':   fracbits = atoi(optarg);    break;
        default:
            fprintf(stderr,"error: %s, unknown option\n", argv[0]);
            usage();
            return 1;
        }
    }

    // validate length
    unsigned int totalbits = intbits + fracbits;
    if (totalbits != 8 && totalbits != 16 && totalbits != 32) {
        fprintf(stderr,"error: %s, invalid total bits (%u), must be 8,16,32\n", argv[0], totalbits);
        exit(-1);
    }

    unsigned int tabsize = 16;
    unsigned int polyorder = 3;

    // coefficients table
    float sine_pwpoly_tab[16][3] = {
        {-0.060522662541018,   1.572056911681783,   0.000000000000000},
        {-0.180985121297741,   1.587105617228409,  -0.000469987617146},
        {-0.299704594399829,   1.616767369563292,  -0.002322714891786},
        {-0.415537748422660,   1.660177846704953,  -0.006389920034732},
        {-0.527369046772096,   1.716057960183830,  -0.013370492257612},
        {-0.634121492911867,   1.782734465474581,  -0.023781856592635},
        {-0.734767002428941,   1.858167007778208,  -0.037915785180656},
        {-0.828336304047983,   1.939981237411416,  -0.055799761507165},
        {-0.913928274244483,   2.025507546203796,  -0.077164923354230},
        {-0.990718615555777,   2.111824899009417,  -0.101421490376865},
        {-1.057967795017763,   2.195809163693769,  -0.127642445077247},
        {-1.115028166269405,   2.274185279619180,  -0.154556083676558},
        {-1.161350206743506,   2.343582549571484,  -0.180547888374104},
        {-1.196487809868082,   2.400592293888733,  -0.203671997319161},
        {-1.220102581317005,   2.441827068940535,  -0.221672366098906},
        {-1.231967097931781,   2.463980625487677,  -0.232013527555895}
    };

    // generate header
    unsigned int i;
    unsigned int j;

    fprintf(fid,"// auto-generated file (do not edit)\n\n");
    fprintf(fid,"// invoked as : ");
    for (i=0; i<argc; i++)
        fprintf(fid,"%s ", argv[i]);
    fprintf(fid,"\n\n");

    fprintf(fid,"#include \"liquidfpm.internal.h\"\n\n");

    // generate sine table
    fprintf(fid,"// sine piece-wise polynomial table\n");
    fprintf(fid,"const %s_t %s_sine_pwpoly_tab[%u][%u] = {\n", qtype,qtype,tabsize,polyorder);
    for (i=0; i<tabsize; i++) {
        fprintf(fid,"    {");
        for (j=0; j<polyorder; j++) {
            fprintf(fid,"0x%.8x", qtype_float_to_fixed(sine_pwpoly_tab[i][j],intbits,fracbits));

            if (j != (polyorder-1))
                fprintf(fid,", ");
        }
        fprintf(fid,"}");

        if ( i == (tabsize-1) )
            fprintf(fid,"\n};\n");
        else
            fprintf(fid,",\n");
    }

    fprintf(fid,"\n\n");

    return 0;
}

