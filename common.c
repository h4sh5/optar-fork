/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#include <stdio.h> /* fprintf */

#include "optar.h"


unsigned data_width;
unsigned data_height;
unsigned width;

/* Properties of the narrow horizontal strip, with crosses */
unsigned narrowheight;
unsigned gapwidth;
unsigned narrowwidth; /* Useful width */
unsigned narrowpixels; /* Useful pixels */

unsigned wideheight;
unsigned widewidth;
unsigned widepixels;

unsigned repheight;
unsigned reppixels;

long totalbits;

unsigned fec_syms;
unsigned netbits;
unsigned usedbits;

unsigned border = 2;
unsigned chalf = 3;
unsigned cpitch = 24;
unsigned xcrosses = 32;
unsigned ycrosses = 46;
unsigned text_width = 13; // has to be 13!! Do not change!

/* initialize rest of the values based on xcrosses and ycrosses */
void init_values(unsigned xcrosses_input, unsigned ycrosses_input) {
	xcrosses = xcrosses_input;
	ycrosses = ycrosses_input;
	data_width = cpitch*(xcrosses-1)+2* chalf;
	data_height =  cpitch*(ycrosses-1)+2* chalf;
	width = 2*border+data_width;
	narrowheight = 2*chalf;
	gapwidth = cpitch-2*chalf;
	narrowwidth = gapwidth*(xcrosses-1);
	narrowpixels = narrowheight*narrowwidth;
	wideheight = gapwidth;
	widewidth = width-2*border;
	widepixels = wideheight*widewidth;
	repheight = narrowheight+wideheight;
	reppixels = widepixels+narrowpixels;
	totalbits = (long)reppixels*(ycrosses-1)+narrowpixels;
	fec_syms = totalbits/FEC_LARGEBITS;
	netbits = fec_syms*FEC_SMALLBITS;
	usedbits = fec_syms*FEC_LARGEBITS;

}


/* Coordinates don't count with the border - 0,0 is upper left corner of the
 * first cross! */
int is_cross(unsigned x, unsigned y)
{
	x%=cpitch;
	y%=cpitch;
	return (x<2*chalf&&y<2*chalf);
}

/* Returns the coords relative to the upperloeftmost cross upper left corner
 * pixel! If you have borders, you have to add them! */
void seq2xy(int *x, int *y, unsigned seq)
{
	unsigned rep; /* Repetition - number of narrow strip - wide strip pair,
			 starting with 0 */

	if (seq>=totalbits){
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
	rep=seq/reppixels;
	seq=seq%reppixels;

	*y=repheight*rep;
	/* Now seq is sequence in the repetition pair */
	if (seq>=narrowpixels){
		/* Second, wide strip of the pair */
		*y+=narrowheight;
		seq-=narrowpixels;
		/* Now seq is sequence in the wide strip */
		*y+=seq/widewidth;
		*x=seq%widewidth;
	}else{
		/* First, narrow strip of the pair */
		unsigned gap; /* Horizontal gap number */
		*x=2*chalf;
		*y+=seq/narrowwidth;
		seq%=narrowwidth;
		/* seq is now sequence in the horiz. line */
		gap=seq/gapwidth;
		*x+=gap*cpitch;
		seq%=gapwidth;
		/* seq is now sequence in the gap */
		*x+=seq;
	}
}

/* Golay codes */
unsigned long golay(unsigned long in)
{
	return golay_codes[in&4095];
}


