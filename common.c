/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#include <stdio.h> /* fprintf */

#include "optar.h"


unsigned g_data_width;
unsigned g_data_height;
unsigned g_width;

/* Properties of the narrow horizontal strip, with crosses */
unsigned g_narrowheight;
unsigned g_gapwidth;
unsigned g_narrowwidth; /* Useful width */
unsigned g_narrowpixels; /* Useful pixels */

unsigned g_wideheight;
unsigned g_widewidth;
unsigned g_widepixels;

unsigned g_repheight;
unsigned g_reppixels;

long g_totalbits;

unsigned g_fec_syms;
unsigned g_netbits;
unsigned g_usedbits;

unsigned g_border = 2;
unsigned g_chalf = 3;
unsigned g_cpitch = 24;
unsigned g_xcrosses = 32;
unsigned g_ycrosses = 46;
unsigned g_text_width = 13; // has to be 13!! Do not change!

/* initialize rest of the values based on xcrosses and ycrosses */
void init_values(unsigned xcrosses_input, unsigned ycrosses_input) {
	g_xcrosses = xcrosses_input;
	g_ycrosses = ycrosses_input;
	g_data_width = g_cpitch*(g_xcrosses-1)+2* g_chalf;
	g_data_height =  g_cpitch*(g_ycrosses-1)+2* g_chalf;
	g_width = 2*g_border+g_data_width;
	g_narrowheight = 2*g_chalf;
	g_gapwidth = g_cpitch-2*g_chalf;
	g_narrowwidth = g_gapwidth*(g_xcrosses-1);
	g_narrowpixels = g_narrowheight*g_narrowwidth;
	g_wideheight = g_gapwidth;
	g_widewidth = g_width-2*g_border;
	g_widepixels = g_wideheight*g_widewidth;
	g_repheight = g_narrowheight+g_wideheight;
	g_reppixels = g_widepixels+g_narrowpixels;
	g_totalbits = (long)g_reppixels*(g_ycrosses-1)+g_narrowpixels;
	g_fec_syms = g_totalbits/FEC_LARGEBITS;
	g_netbits = g_fec_syms*FEC_SMALLBITS;
	g_usedbits = g_fec_syms*FEC_LARGEBITS;

}


/* Coordinates don't count with the border - 0,0 is upper left corner of the
 * first cross! */
int is_cross(unsigned x, unsigned y)
{
	x%=g_cpitch;
	y%=g_cpitch;
	return (x<2*g_chalf&&y<2*g_chalf);
}

/* Returns the coords relative to the upperloeftmost cross upper left corner
 * pixel! If you have borders, you have to add them! */
void seq2xy(int *x, int *y, unsigned seq)
{
	unsigned rep; /* Repetition - number of narrow strip - wide strip pair,
			 starting with 0 */

	if (seq>=g_totalbits){
		/* Out of range */
		*x=-1;
		*y=-1;
		return;
	}
	/* We are sure we are in range. Document structure:
	 * - narrow strip (between top row of crosses), height is
	 *   2*chalf
	 * - wide strip, height is cpitch-2*chalf
	 * - the above repeats (YCROSSES-1)-times
	 * - narrow strip 
	 */
	rep=seq/g_reppixels;
	seq=seq%g_reppixels;

	*y=g_repheight*rep;
	/* Now seq is sequence in the repetition pair */
	if (seq>=g_narrowpixels){
		/* Second, wide strip of the pair */
		*y+=g_narrowheight;
		seq-=g_narrowpixels;
		/* Now seq is sequence in the wide strip */
		*y+=seq/g_widewidth;
		*x=seq%g_widewidth;
	}else{
		/* First, narrow strip of the pair */
		unsigned gap; /* Horizontal gap number */
		*x=2*g_chalf;
		*y+=seq/g_narrowwidth;
		seq%=g_narrowwidth;
		/* seq is now sequence in the horiz. line */
		gap=seq/g_gapwidth;
		*x+=gap*g_cpitch;
		seq%=g_gapwidth;
		/* seq is now sequence in the gap */
		*x+=seq;
	}
}

/* Golay codes */
unsigned long golay(unsigned long in)
{
	return golay_codes[in&4095];
}


