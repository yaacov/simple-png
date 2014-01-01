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

unsigned int
spng_dec_read_bit(FILE *fp);

unsigned int
spng_dec_read_bits(FILE *fp, unsigned int length);

unsigned int
spng_dec_read_bitsR(FILE *fp, unsigned int length);

unsigned int
spng_dec_read_literal(FILE *fp);

unsigned int
spng_dec_read_length(FILE *fp, unsigned int bit);

unsigned int
spng_dec_read_size(FILE *fp, unsigned int *w, unsigned int *h);

unsigned int
spng_dec_read_palet(FILE *fp);

unsigned int
spng_dec_go_to_image_data(FILE *fp);

unsigned int
spng_dec_literal_to_rgb(FILE *fp, unsigned char color, 
    unsigned char *r, unsigned char *g, unsigned char *b);

