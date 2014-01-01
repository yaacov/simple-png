/*  spng - simple-png utilities
 *
 *  Copyright (C) 2011  Yaacov Zamir <kobi.zamir@gamil.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <stdio.h>
#include <string.h>
 
#include "spng_dec.h"

// to compile:
//      cc spng_dec.c png2ppm.c -o png2ppm

int
write_ppm_header(FILE* fp, unsigned int w, unsigned int h) {
    int len;
    unsigned char ptr[16];
    
    // create width and height string
    sprintf(ptr, "%d %d\n", w, h);
    len = strlen(ptr);
    
    // weite ppm header
    fwrite("P6\n", 1, 3, fp);
    fwrite(ptr, 1, len, fp);
    fwrite("255\n", 1, 4, fp);
    
    return 1;
}

int
write_ppm_pixel(FILE* fp, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char ptr[16];
    
    // write P6 pixel
    ptr[0] = r;
    ptr[1] = g;
    ptr[2] = b;
    
    fwrite(ptr, 1, 3, fp);
    
    return 1;
}

int
usage() {
    printf("spng2ppm png_file_name\n\n");
    printf("  reads simple-png input file and\n");
    printf("  writes ppm output to out.ppm file\n\n");
    
    return 1;
}

int
main(int argc, char *argv[]) {
    unsigned char r, g, b;
    unsigned int i, x, y;
    unsigned int w, h;
    unsigned int lit;
    
    FILE *fp = NULL;
    FILE *fpppm = NULL;
    
    // open png file for reading
    if (argc > 1) {
        fp=fopen(argv[1], "r");
    }
    
    if (!fp) {
        usage();
        return 2;
    }
    
    // open the out ppm file
    fpppm=fopen("out.ppm", "w");
    if (!fpppm) {
        usage();
        return 2;
    }
    
    // read width and height from header
    spng_dec_read_size(fp, &w, &h);
    
    // weite ppm header
    write_ppm_header(fpppm, w, h);
    
    // load the color palet form png file
    spng_dec_read_palet(fp);
    
    // set the png file pointer to start of image data
    spng_dec_go_to_image_data(fp);
    
    // read 3 bits, means last chank of data
    spng_dec_read_bits(fp, 3);
    
    // read lines
    y = 0;
    while (y < h) {
        // read compresion type - must be zero
        spng_dec_read_literal(fp);
        
        x = 0;
        while (x < w) {
            // read a literal (color or length)
            lit = spng_dec_read_literal(fp);
            
            // < 256 , a colot literal
            if (lit < 256) {
                // parse the rgb8 palet code to rgb32 
                spng_dec_literal_to_rgb(fp, lit, &r, &g, &b);
                
                // write color to ppm file
                x++;
                write_ppm_pixel(fpppm, r, g, b);
            // > 256 , a length literal
            } else {
                // read the extra length bits from file
                lit = spng_dec_read_length(fp, lit);
                x+= lit;
                
                // read the 5 distance bits, must be zero -> 1 byte distance
                spng_dec_read_bits(fp, 5);
                
                // write repeated color to ppm file
                for (i = 0; i < lit; i++) {
                    write_ppm_pixel(fpppm, r, g, b);
                }
            }
        }
        y++;
    }
    
    // close input and output files
    fclose(fpppm);
    fclose(fp);
    
    return 1;
}

