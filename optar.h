/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#ifndef OPTAR_H
#define OPTAR_H

#include <stdint.h>

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

extern uint32_t border; /* In pixels. Thickness of the border */
extern uint32_t chalf; /* Size of the cross half. Size of the cross is CHALF*2 x CHALF*2.
		   */
extern uint32_t cpitch; /* Distance between cross centers */

// XXX X-Y crosses reduced due to too much data loss (originally 65x93 / 200KB per page, at half size 32x46 which encodes 1/4 of data / 50KB a page)
/* XCROSSES A4 65, US Letter 67. (originally)*/
/* Number of crosses horizontally */
extern uint32_t xcrosses;

/* YCROSSES A4 93, US Letter 87. (originally)*/
/* Number of crosses vertically */
extern uint32_t ycrosses;

extern uint32_t data_width; /* The rectangle occupied bythe data and crosses */
extern uint32_t data_height;
extern uint32_t width; /* In pixels, including the border */
/* In pixels, including the border and the label */

extern uint32_t text_width; /* Width of a single letter */

/* Definitions for seq2xy */

/* Properties of the narrow horizontal strip, with crosses */
extern uint32_t narrowheight;
extern uint32_t gapwidth;
extern uint32_t narrowwidth; /* Useful width */
extern uint32_t narrowpixels; /* Useful pixels */

/* Properties of the wide horizontal strip, without crosses */
extern uint32_t wideheight;
extern uint32_t widewidth;
extern uint32_t widepixels;

/* Amount of raw payload pixels in one narrow-wide strip pair */
extern uint32_t repheight;
extern uint32_t reppixels;


/* Total bits before hamming including the unused */
extern uint64_t totalbits;

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
extern uint32_t fec_syms;
extern uint32_t netbits; /* Net payload bits */
extern uint32_t usedbits; /* Used raw bits to store
						     Hamming symbols */

/* Functions from common.c */
extern uint64_t parity(uint64_t in);
extern int is_cross(uint32_t x, uint32_t y);
extern void seq2xy(int *x, int *y, uint32_t seq);

/* Counts number of '1' bits */
uint32_t ones(uint64_t in);

/* Golay codes */
uint64_t golay(uint64_t in);
extern uint64_t golay_codes[4096];
extern void init_values(uint32_t xcrosses_input, uint32_t ycrosses_input);
#endif