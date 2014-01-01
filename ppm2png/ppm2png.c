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

#include "deflate.h"
#include "png_enc.h"

// to compile:
//      cc deflate.c png_enc.c  ppm2png.c -o ppm2png

int
read_ppm_int(FILE *fp) {
    int i = 0;
    unsigned char ch;
    
    fread(&ch, 1, 1, fp);
    
    // read none number chars
    while (ch < 48 || ch > 57) {
        // if line has '#' ignore until end of line
        if (ch == '#') read_ppm_line(fp);
    
        fread(&ch, 1, 1, fp);
    }
    
    // read number
    while (ch >= 48 && ch <= 57) {
        i = i * 10 + (ch - 48);
        fread(&ch, 1, 1, fp);
    }
    
    // go one char back
    fseek(fp, -1, SEEK_CUR);
    
    return i;
}

int
read_ppm_line(FILE *fp) {
    int i = 0;
    unsigned char ch;
    
    fread(&ch, 1, 1, fp);
    
    while (ch != '\n') fread(&ch, 1, 1, fp);
    
    return i;
}

unsigned char rgb_to_index(unsigned char r, unsigned char g, unsigned char b) {
    
    r = (r & 0xc0);
    g = (g & 0xe0) >> 2;
    b = (b & 0xe0) >> 5;
    
    return r | g | b;
}

int
usage() {
    printf("ppm2png ppm_file_name\n\n");
    printf("  reads ppm input file and\n");
    printf("  writes simple png output to out.png file\n\n");
    
    return 1;
}

int
main(int argc, char *argv[]) {
    unsigned char r, g, b;
    unsigned int ppm_number;
    unsigned int i, x, y;
    unsigned int w, h;
    unsigned int lit;
    unsigned long crc = 0;
    unsigned long adler_crc = 1L;
    
    unsigned char deflate_file[0x20000];
    unsigned long len;
    
    FILE *fp = NULL;
    FILE *fpppm = NULL;
    
    // open png file for reading
    if (argc > 1) {
        fpppm=fopen(argv[1], "rb");
    }
    
    if (!fpppm) {
        usage();
        return 2;
    }
    
    // open the out ppm file
    fp=fopen("out.png", "wb");
    if (!fp) {
        usage();
        return 2;
    }
    
    // read ppm header
    ppm_number = read_ppm_int(fpppm); // P3 or P6
    w = read_ppm_int(fpppm); // width
    h = read_ppm_int(fpppm); // height
    i = read_ppm_int(fpppm); // 255 color depth
    read_ppm_line(fpppm); // read to the end of the line
    
    // write png maigc, header and palet
    write_png_magic(fp);
    crc = write_png_header(fp, w, h);
    crc = write_png_palet(fp, crc);
    
    // start writing into the deflate_file
    deflate_open((unsigned char *)deflate_file);
    
    // start last group (use 0x02 for not last group)
    deflate_put_bits(3, 0x03); 
    
    // read ppm data and build deflate file
    for (y = 0; y < h; y++){
        
        // init a new line of pixels
        // compresion type = 0, simple repeat
        adler_crc = deflate_put_pixel(1, 0, 0, 0, adler_crc);
        
        for (x = 0; x < w; x++) {
            // read ppm r g b pixel
            // P3 file - ascii
            if (ppm_number == 3) {
                r = read_ppm_int(fpppm);
                g = read_ppm_int(fpppm);
                b = read_ppm_int(fpppm);
            // P6 file - binary
            } else {
                fread(&r, 1, 1, fpppm);
                fread(&g, 1, 1, fpppm);
                fread(&b, 1, 1, fpppm);
            }
            
            lit = rgb_to_index(r, g, b);
            
            // add a pixel to the deflate stream
            adler_crc = deflate_put_pixel(0, lit, x, w, adler_crc);
        }
    } 
    
    // FIXME: some decoders want extra literals
    for (i = 0; i < 255; i)
        adler_crc = deflate_put_pixel(0, 0, x, w, adler_crc);
    
    deflate_put_bits(7, 0x00); // end of group
    
    // close deflate file and get it's length
    len = deflate_close() - (unsigned char *)deflate_file;
    
    // write png data from deflate file to ouput file
    write_png_data(fp, crc, adler_crc, deflate_file, len);
    
    // write png end 
    crc = write_png_end(fp, crc);
    
    // close input and output files
    fclose(fp);
    fclose(fpppm);
    
    return 1;
}

