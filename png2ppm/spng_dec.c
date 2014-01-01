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

#include "spng_dec.h"

static unsigned char bits = 0;
static unsigned char byte[4];
static unsigned char palet[256 * 3];

unsigned int
spng_dec_read_bit(FILE *fp) {
    unsigned int output_bit;
    
    if (bits % 8 == 0) {
        fread(byte, 1, 1, fp);
        bits = 0;
    }
    
    output_bit = (byte[0] >> bits++) & 0x01;
    
    //printf("%d", output_bit);
    return output_bit;
}

unsigned int
spng_dec_read_bits(FILE *fp, unsigned int length) {
    unsigned int output = 0;
    unsigned int i;
    
    for (i = 0; i < length; i++) {
        output |= (spng_dec_read_bit(fp) << i);
    }
    
    return output;
}

unsigned int
spng_dec_read_bitsR(FILE *fp, unsigned int length) {
    unsigned int output = 0;
    unsigned int i;
    
    for (i = 0; i < length; i++) {
        output = (output << 1) | spng_dec_read_bit(fp);
    }
    
    return output;
}

unsigned int
spng_dec_read_literal(FILE *fp) {
    unsigned int bit;
    
    // read first 7 bits
    bit = spng_dec_read_bitsR(fp, 7);
    
    // if less then 24 it's a 7 bit literal
    if (bit < 24)
        return bit + 256;
    
    // it's 8bit read one more bit
    bit = (bit << 1) | spng_dec_read_bit(fp);
    
    // if less then 192 it's a 8 bit literal step 1
    if (bit < 192)
        return bit - 48;
    
    // if less then 196 it's a 8 bit literal step 2
    if (bit < 200)
        return bit - 192 + 280;
    
    // it's 9bit read one more bit
    bit = (bit << 1) | spng_dec_read_bit(fp);
    
    return bit - 400 + 144;
}

unsigned int
spng_dec_read_length(FILE *fp, unsigned int bit) {
    unsigned int extra_bit;
    
    if (bit == 285)
        return 258;
    
    // 254-264
    if (bit < 265)
        return bit - 254;
    
    // read one extra bit
    extra_bit = spng_dec_read_bit(fp);
        
    // 265-269
    if (bit < 269)
        return 11 + (bit - 265) * 2 + extra_bit;
    
    // read one more extra bit
    extra_bit += spng_dec_read_bit(fp) << 1;
    
    // 269-273
    if (bit < 273)
        return 19 + (bit - 269) * 4 + extra_bit;
    
    // read one more extra bit
    extra_bit += spng_dec_read_bit(fp) << 2;
    
    // 273-277
    if (bit < 277)
        return 35 + (bit - 273) * 8 + extra_bit;
    
    // read one more extra bit
    extra_bit += spng_dec_read_bit(fp) << 3;
    
    // 277-281
    if (bit < 281)
        return 67 + (bit - 277) * 16 + extra_bit;
    
    // read one more extra bit
    extra_bit += spng_dec_read_bit(fp) << 4;
    
    // 281-285
    if (bit < 285)
        return 131 + (bit - 281) * 32 + extra_bit;
    
    return bit;
}

unsigned int
spng_dec_read_size(FILE *fp, unsigned int *w, unsigned int *h) {
    unsigned char ptr[16];
    
    // read width and height from header
    // NOTE: we assume the header starts at byte 12 + 4
    fseek(fp, 16, SEEK_SET);
    
    fread(ptr, 1, 4, fp);
    *w = (((ptr[0] << 8) + ptr[1] << 8) + ptr[2] << 8) + ptr[3];
    
    fread(ptr, 1, 4, fp);
    *h = (((ptr[0] << 8) + ptr[1] << 8) + ptr[2] << 8) + ptr[3];
    
    return 1;
}

unsigned int
spng_dec_read_palet(FILE *fp) {

    // NOTE: we assume the palet starts at byte 37 + 4
    fseek(fp, 41, SEEK_SET);
    
    // 256 sets of rgb colors
    fread(palet, 1, 256 * 3, fp);
    
    return 1;
}

unsigned int
spng_dec_go_to_image_data(FILE *fp) {

    // NOTE: we assume the compressed data starts at byte 823
    fseek(fp, 823, SEEK_SET);
    
    return 1;
}

unsigned int
spng_dec_literal_to_rgb(FILE *fp, unsigned char color, 
    unsigned char *r, unsigned char *g, unsigned char *b) {

    *r = palet[color * 3 + 0];
    *g = palet[color * 3 + 1];
    *b = palet[color * 3 + 2];
    
    return 1;
}

