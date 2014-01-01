#!/usr/bin/env python
# -*- coding:utf-8 -*-

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

# Copyright (C) 2011 Yaacov Zamir (2011) <kobi.zamir@gmail.com>
# Authors: 
#    Yaacov Zamir (2011) <kobi.zamir@gmail.com>

import sys
import zlib
from struct import pack, unpack
from collections import Counter

img_data = None
bits = 0
byte = 0
    
def writeString(data, string):
    
    for c in string:
        data += pack('B', ord(c))
    return data
    
def writeLong(data, l):
    
    data += pack('>i', l)
    return data
    
def writeWord(data, h):
    
    data += pack('>h', h)
    return data
    
def writeByte(data, b):
    
    data += pack('B', b & 0xff)
    return data
    
def writeBit(data, b):
    
    global bits
    global byte
    
    b = [0,1][b != 0]
    
    bits += 1
    byte = (byte >> 1) | (b << 7)
    if bits == 8:
        data = writeByte(data, byte)
        bits = 0
    return data
    
def writeBits(data, num, val):

    while num > 0:
        data = writeBit(data, val & 1)
        val >>= 1
        num -= 1
    return data
    
def writeBitsR(data, num, val):

    mask = 1 << (num - 1)
    while num > 0:
        data = writeBit(data, val & mask)
        mask >>= 1
        num -= 1
    return data
    
def flushBits(data):
    
    global bits
    
    while bits:
        data = writeBit(data, 0)
    return data
    
def beginChunk(data, name, length):
    
    data = writeLong(data, length)
    data = writeString(data, name)
    
    return data
    
def endChunk(data, crc):
    
    crc = zlib.crc32(data[4:], crc)
    data = writeLong(data, crc)
    
    return data, crc

def PutLiteral(data, val):
    if (val < 144) :
        val = 48 + val
        data = writeBitsR(data, 8, val)
    
    elif (val < 256) :
        val = 400 + (val - 144)
        data = writeBitsR(data, 9, val)
    
    elif (val < 280) :
        data = writeBitsR(data, 7, val - 256)
    
    elif (val < 288) :
        data = writeBitsR(data, 8, 192 + (val - 280))
    
    return data

def PutLength(data, length):
    if (length > 2 and length < 11) :
        data = PutLiteral(data, 254 + length)
    
    elif (length < 19) :
        data = PutLiteral(data, 265 + (length - 11) / 2);
        data = writeBits(data, 1, (length - 3) % 2)
    
    elif (length < 35) :
        data = PutLiteral(data, 269 + (length - 19) / 4);
        data = writeBits(data, 2, (length - 3) % 4)
    
    elif (length < 67) :
        data = PutLiteral(data, 273 + (length - 35) / 8);
        data = writeBits(data, 3, (length - 3) % 8)
    
    elif (length < 131) :
        data = PutLiteral(data, 277 + (length - 67) / 16);
        data = writeBits(data, 4, (length - 3) % 16)
    
    elif (length < 258) :
        data = PutLiteral(data, 281 + (length - 131) / 32);
        data = writeBits(data, 5, (length - 3) % 32)
    
    elif (length == 258) :
        data = PutLiteral(data, 285)

    return data

def median_cut(color, last_cut):
    global img_data
    
    length = len(img_data)
    
    # cut all current cuts
    current_cuts = last_cut
    for cut in range(current_cuts + 1):
        # update last_cut number
        last_cut = last_cut + 1
        
        # find the median value in cut
        median = 0
        pixels = [(p >> (color * 8)) & 0xff for p in img_data if (p >> 24) == cut]
        if len(pixels) > 0:
            pixels.sort()
            median = pixels[len(pixels) / 2]
        
        # update the cut number for values above the median
        for i in range(length):
            p = img_data[i]
            if (p >> 24) == cut and ((p >> (color * 8)) & 0xff) > median:
                img_data[i] = img_data[i] & 0x00ffffff | (last_cut << 24)
    
    return last_cut
    
def cut_to_color(cut):
    global img_data
    
    # get all reds in cut and avarage them
    pixels = [(p >> 16) & 0xff for p in img_data if (p >> 24) == cut]
    
    # check tnat we have pixels in this cut
    if len(pixels) == 0:
        return [0, 0, 0]
    
    r = sum(pixels) / len(pixels)

    # get all green in cut and avarage them
    pixels = [(p >> 8) & 0xff for p in img_data if (p >> 24) == cut]
    g = sum(pixels) / len(pixels)
    
    # get all blue in cut and avarage them
    pixels = [p & 0xff for p in img_data if (p >> 24) == cut]
    b = sum(pixels) / len(pixels)
    
    # return color
    return [r, g, b]
    
def histogram(img, qapp):
    global img_data
    
    w = img.width()
    h = img.height()
    
    cnt = Counter()
    img_data = []
    for y in range(h):
        qapp.processEvents()
        img_data += unpack('%dI' % w, img.scanLine(y))
    
    # init image cuts to 0
    length = len(img_data)
    
    for i in range(length):
        img_data[i] = img_data[i] & 0x00ffffff
    
    # count colors
    cnt.update(img_data)
    
    # init mumber of colors and the colors palet
    num_of_colors = len(list(cnt))
    palet_arr = {}
    
    if num_of_colors <= 256:
        # use indexed colors
        palet_dict = {}
        c = 0
        
        # build palet
        for i in range(256):
            if i < num_of_colors:
                c = cnt.most_common()[i][0]
            palet_dict[c] = i
            palet_arr[i] = [(c >> 16) & 0xff, (c >>  8) & 0xff, c & 0xff]
        
        # update pixel color index in image data
        for i in range(length):
            c = img_data[i]
            img_data[i] = (c & 0x00ffffff) | (palet_dict[c] << 24)
            
    else:
        # use median cuts ( 2^8 cuts )
        last_cut = 0
        for i in range(8):
            qapp.processEvents()
            last_cut = median_cut(i % 3, last_cut)
        
        # build palet
        for i in range(256):
            qapp.processEvents()
            palet_arr[i] = cut_to_color(i)
    
    return palet_arr
    
def writeSPangImage(fileName, img, qapp):
    global img_data
    global bits
    global byte
    
    # recreate the color palet
    palet_arr = histogram(img, qapp)
    
    bits = 0
    byte = 0
    
    f = open(fileName, 'wb')
    
    w = img.width()
    h = img.height()
    
    magic = pack("8B", 137, 80, 78, 71, 13, 10, 26, 10)
    f.write(magic)
    
    header = ''
    header = beginChunk(header, "IHDR", 0x0d);
    header = writeLong(header, w)    # width 
    header = writeLong(header, h)    # height
    header = writeByte(header, 8)    # bit depth 
    header = writeByte(header, 3)    # color type 
    header = writeByte(header, 0)    # compression 
    header = writeByte(header, 0)    # filter 
    header = writeByte(header, 0)    # interlace 
    
    header, crc = endChunk(header, 0)
    
    f.write(header)
    
    palet = ''
    palet = beginChunk(palet, "PLTE", 256 * 3);
    for c in range(256):
        palet = writeByte(palet, palet_arr[c][0])
        palet = writeByte(palet, palet_arr[c][1])
        palet = writeByte(palet, palet_arr[c][2])
    
    palet, crc = endChunk(palet, 0)
    
    f.write(palet)
    
    data = ''
    raw_data = ''
    
    # start last group (use 0x02 for not last group)
    data = writeBits(data, 3, 0x03)
    
    for y in range(h):
        qapp.processEvents()
        
        # 0, simple repeat compression for this line
        data = PutLiteral(data, 0);
        last_color = 256
        length = 0
        for x in range(w):
            # get color 
            color = (img_data[y * w + x] >> 24) & 0xff
            
            if color != last_color or length > 256 or x == (w - 1):
                
                # write the repeats of last color
                if length > 2:
                    data = PutLength(data, length);
                    data = writeBits(data, 5, 0); # distance 1
                else:
                    for i in range(length):
                        data = PutLiteral(data, last_color);
                
                # write the new color
                data = PutLiteral(data, color);
                last_color = color
                length = 0;
            else:
                length += 1
    
    # FIXME: some decoders want an extra literal
    data = PutLiteral(data, 0);
    data = PutLength(data, 255);
    
    # end of group
    data = writeBits(data, 7, 0)
    data = flushBits(data);
    
    adler32 = zlib.adler32(raw_data)
    data = writeLong(data, 0)
    
    data_len = len(data)
    
    idat = ''
    idat = beginChunk(idat, "IDAT", 2 + data_len)
    idat = writeWord(idat, ((0x0800 + 30) / 31) * 31 ) # compression method
    
    # start last group (use 0x02 for not last group)
    idat = writeBits(idat, 3, 0x03); 
    idat += data
    
    # close data chank - write crc
    idat, crc = endChunk(idat, 0)

    f.write(idat)
    
    # write end chank
    end = ''
    end = beginChunk(end, "IEND", 0);
    end, crc = endChunk(end, 0)
    
    f.write(end)
    
    f.closed
    return

def writePpm(fileName, img):
    
    w = img.width()
    h = img.height()
    
    with open(fileName, 'wb') as f:
        f.write('P6\n')
        f.write('%d %d\n' % (w, h))
        f.write('255\n')
        
        for i in range(h):
            for j in range(w):
                # Format_ARGB32 => AARRGGBB
                pixel = img.pixel(j, i)
                r = (pixel >> 16) & 0xff
                g = (pixel >>  8) & 0xff
                b = (pixel      ) & 0xff
                
                f.write(pack('BBB', r, g, b))
                
        f.closed
    return

