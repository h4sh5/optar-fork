# Optar

<img src=docs/optar_strip.png>

**This repo is a maintenance and modernization fork of Optar - OPTical ARchiver**. It was written around 2007 by Twibright Labs. Thanks to the magical ability of projects written in C to pretty much work forever, it's has survived bitrot (and even works in 2025 on a apple silicone macbook).

The goal of this repo is to provide more visibility of this project, while working towards a browser-only optar web application that can optar/unoptar files on any browser without a server or the need to compile any of the code yourself.

## What is Optar?

Optar stands for OPTical ARchiver. It's a codec for encoding data on paper or free software 2D barcode in other words. Optar fits 200kB on an A4 page, then you print it with a laser printer (at 600dpi+ printing, 1200dpi+ scanning for best results). If you want to read the recording, scan it with a scanner and feed into the decoder program. A practical level of reliability is ensured using forward error correction code (FEC). Automated processing of page batches facilitates storage of files larger than 200kB.

## Usage

### Encoding (writing)

`./optar <path to file> <file label> [xcrosses] [ycrosses]`

default value for xcrosses/ycrosses if unspecified is 32 46

for example:

`./optar path/to/file.txt file.label 46 69`

Some example values for xcrosses and ycrosses (smaller = less data per page, more pages needed to encode data, but less room for error; bigger = more data per page, less pages needed, more room for error).

Recommend using smaller x/y values so that crappier printers and scanners can still do the job.

Example values:

```
65 93 - 200kB encoded per A4 page (6 pages needed for 1MB)
48 70 - 100kB per A4 page (10 pages for 1MB)
32 46 - 50kB per A4 page (23 pages for 1MB) - default
```

Old README:

```
Twibright Optar
---------------

This is a program to store data on paper using a 600dpi b/w laser printer and a
600+ dpi scanner.

You need to install ImageMagick so that the resulting .pgm image can be converted into
PostScript with the right dimensions (so each pixel is 3x3 600dpi pixels so that
there is no unnecessary jitter).

Make sure you have libpng installed and if you type "libpng-config" on the
commandline, there's a program which prints something.

Compile with "make". Become root (su -) and type "make install". Now you have
optar, unoptar and pgm2ps installed on your system in /usr/local/bin. Later
you can uninstall by typing "make uninstall" the same way as you typed
"make install".

Encoding (writing)
------------------
Run

./optar other_guys.ogg other_guys.ogg  

which will produce files:

other_guys.ogg_0001.pgm
other_guys.ogg_0002.pgm
other_guys.ogg_0003.pgm
other_guys.ogg_0004.pgm
other_guys.ogg_0005.pgm
other_guys.ogg_0006.pgm

Now convert them into PostScript using the included pgm2ps tool:
./pgm2ps *.pgm

Print them using a 600dpi (or more) laser printer. Inkjet or dot matrix was
never tested and will not probably work at the pre-defined data density. See
"Changing the format" below.

Please note that the file will be padded by zeroes and the original length will
be lost. Pack your data with tar if you store data that are sensitive to this.

Decoding (reading)
------------------

Clean and polish the scanner glass with rubbing alcohol and paper towel. Put
yellow pages on the scanner lid to get sharper picture *). Insert the
page so that the text on the bottom is upright. Scan the pages into PNG (not
JPEG!) on 600dpi (or 1200dpi, slightly better), grayscale, with gamma
correction turned of (set to 1.0). Some scanners have "True Gray" vs. "Gray
(Error Diffusion)", take the True Gray.

->   PNG, 600 DPI, grayscale, gamma correction turned off   <-

scan_0001.png
scan_0002.png
scan_0003.png
scan_0004.png
scan_0005.png
scan_0006.png

Read the number sequence (format specification) from any of the papers and feed
it as 1st argument to the optar, 2nd argument is the filename part before the
underscore:

unoptar 0-65-93-24-3-1-2-24 scan > out.ogg

Then play out.ogg with mplayer. You should get first about 41 seconds from the
Ogg Vorbis file.

*) In the scanner I tried (Canoscan), the lid didn't seem to be heavy enough to
press the paper down completely - there were blurry spots in the picture.
Without yellow pages I got 526 reparable bad bits bad from 3.2 million. With
yellow pages the blurry spots were much sharper and I got only 261 reparably
bad bits!

Please note the data are padded with zeroes so the original information
about file length is lost. If your data format doesn't like this then first
pack your data with tar.

A4 <-> US Letter
----------------
Change the convert parameters in pgm2ps (see comments). Change XCROSSES
and YCROSSES in optar.h (see comments). Recompile. Then you can use US Letter
instead of A4.

Changing the format
-------------------
If your printer is low quality and you are getting irreparable bits, you can
try to format the media to lower capacity.  Unfortunately, setting by
commandline is not implemented yet.  Change XCROSSES and YCROSSES in optar.h to
lower values which yields bigger pixels and lower capacity per page, but higher
reliability.  Make sure they are in roughly the same proportion as before,
otherwise you get nonsquare pixels and unnecessary waste of channel capacity.

You can also change the decoding parameters in unoptar.c (look for MAGIC
CONSTANTS) in attempt to read a difficult recording: unsharp_mask,
unsharp_dist, sync_white_cut, white_cut, minmax_filter, pixel_blur, cross_trim.

Future improvement
==================
- manpage could be written for optar and unoptar
- commandline help (-h) could be written for optar and unoptar
- the format could be made configurable. Now it's stored in the optar.h
- the magic constants could be changed by commandline options. Now they are
  stored in unoptar.c.
- Golay code decoding could be rewritten faster, using a sophisticated
  algorithm (Kasami algorithm?)
- Easy support for multiple pages per page, so it can be read by a digital
  camera. Currently it cannot since digital camera blurs at the sides of
  the picture.

(c) GPL 2007 Karel 'Clock' Kulhavy of Twibright Labs
See COPYING for the text of the GPL license.
e-mail: clock (at) twibright (dot) com
Twibright Optar homepage: http://ronja.twibright.com/optar/
```