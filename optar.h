/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#ifndef OPTAR_H
#define OPTAR_H

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

extern unsigned g_border; /* In pixels. Thickness of the border */
extern unsigned g_chalf; /* Size of the cross half. Size of the cross is g_chalf*2 x g_chalf*2.
		   */
extern unsigned g_cpitch; /* Distance between cross centers */

// XXX X-Y crosses reduced due to too much data loss (originally 65x93 / 200KB per page, at half size 32x46 which encodes 1/4 of data / 50KB a page)
/* g_xcrosses A4 65, US Letter 67. (originally)*/
/* Number of crosses horizontally */
extern unsigned g_xcrosses;

/* g_ycrosses A4 93, US Letter 87. (originally)*/
/* Number of crosses vertically */
extern unsigned g_ycrosses;

extern unsigned g_data_width; /* The rectangle occupied bythe data and crosses */
extern unsigned g_data_height;
extern unsigned g_width; /* In pixels, including the g_border */
/* In pixels, including the g_border and the label */

extern unsigned g_text_width; /* Width of a single letter */

/* Definitions for seq2xy */

/* Properties of the narrow horizontal strip, with crosses */
extern unsigned g_narrowheight;
extern unsigned g_gapwidth;
extern unsigned g_narrowwidth; /* Useful width */
extern unsigned g_narrowpixels; /* Useful pixels */

/* Properties of the wide horizontal strip, without crosses */
extern unsigned g_wideheight;
extern unsigned g_widewidth;
extern unsigned g_widepixels;

/* Amount of raw payload pixels in one narrow-wide strip pair */
extern unsigned g_repheight;
extern unsigned g_reppixels;


/* Total bits before hamming including the unused */
extern long g_totalbits;

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
extern unsigned g_fec_syms;
extern unsigned g_netbits; /* Net payload bits */
extern unsigned g_usedbits; /* Used raw bits to store
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
extern void init_values(unsigned g_xcrosses_input, unsigned g_ycrosses_input);
#endif