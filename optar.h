/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#ifndef OPTAR_H
#define OPTAR_H

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

extern unsigned border; /* In pixels. Thickness of the border */
extern unsigned chalf; /* Size of the cross half. Size of the cross is CHALF*2 x CHALF*2.
		   */
extern unsigned cpitch; /* Distance between cross centers */

// XXX X-Y crosses reduced due to too much data loss (originally 65x93 / 200KB per page, at half size 32x46 which encodes 1/4 of data / 50KB a page)
/* XCROSSES A4 65, US Letter 67. (originally)*/
/* Number of crosses horizontally */
extern unsigned xcrosses;

/* YCROSSES A4 93, US Letter 87. (originally)*/
/* Number of crosses vertically */
extern unsigned ycrosses;

extern unsigned data_width; /* The rectangle occupied bythe data and crosses */
extern unsigned data_height;
extern unsigned width; /* In pixels, including the border */
/* In pixels, including the border and the label */

extern unsigned text_width; /* Width of a single letter */

/* Definitions for seq2xy */

/* Properties of the narrow horizontal strip, with crosses */
extern unsigned narrowheight;
extern unsigned gapwidth;
extern unsigned narrowwidth; /* Useful width */
extern unsigned narrowpixels; /* Useful pixels */

/* Properties of the wide horizontal strip, without crosses */
extern unsigned wideheight;
extern unsigned widewidth;
extern unsigned widepixels;

/* Amount of raw payload pixels in one narrow-wide strip pair */
extern unsigned repheight;
extern unsigned reppixels;


/* Total bits before hamming including the unused */
extern long totalbits;

/* Hamming codes with parity */
#define FEC_ORDER 1 /* Can be 2 to 5 inclusive. 
			   5 is 26/32,
			   4 is 11/16,
			   3 is 4/8,
			   2 is 4/1
			   1 is golay codes */
#if FEC_ORDER==1
/* Golay */
#define FEC_LARGEBITS 24
#define FEC_SMALLBITS 12
#else
/* Hamming */
#define FEC_LARGEBITS (1<<FEC_ORDER)
#define FEC_SMALLBITS (FEC_LARGEBITS-1-FEC_ORDER)
#endif

/* Hamming net channel capacity */
extern unsigned fec_syms;
extern unsigned netbits; /* Net payload bits */
extern unsigned usedbits; /* Used raw bits to store
						     Hamming symbols */

/* Functions from common.c */
extern unsigned long parity(unsigned long in);
extern int is_cross(unsigned x, unsigned y);
extern void seq2xy(int *x, int *y, unsigned seq);

/* Counts number of '1' bits */
unsigned ones(unsigned long in);

/* Golay codes */
unsigned long golay(unsigned long in);
extern unsigned long golay_codes[4096];
extern void init_values(unsigned xcrosses_input, unsigned ycrosses_input);
#endif