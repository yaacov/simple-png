simple-png
==========

SPNG, simple-PNG image utilities
--------------------------------
simple-png is a PNG image sub format,

SPNG files are PNG files, that are easy to encode and decode using minimal cpu and memory resources, this is nice for embedded devices with little cpu, memory and data storage resources.
SPNG files

SPNG files are PNG files that contain
-------------------------------------
    png magic
    png header: bit depth set to 8, and color type set to 3
    png color pallet : 256 rgb color triplets
    png data : deflate data compressed using simple RLE (e.g deflate distance is always set to 1)
    png ending 

CRC calculation
---------------
Calculating crc is not simple.

    Decoder: simple-png decoder just ignore adler-crc and crc calculation.
    Encoder: simple-png encoder try to calculte crc. 

utiliteis
---------
    png2ppm - decode a simple-png file and write the output to ppm.
    ppm2png - decode a ppm file and write the output to simple-png.
    any2png - a pyside qt GUI for writing image output to simple-png. 

reference
---------
  * [http://en.wikipedia.org/wiki/Portable_Network_Graphics PNG image file format]
  * [http://netpbm.sourceforge.net/doc/ppm.html PPM image file format]
  * [http://www.libpng.org/pub/png/libpng.html libpng library]
  * [http://tools.ietf.org/html/rfc1951 Deflate rfc1951]
  * [http://www.pyside.org/ PySide QT Python wrapper]

