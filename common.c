/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#include <stdio.h> /* fprintf */
#include <stdint.h>
#include "optar.h"


uint32_t data_width;
uint32_t data_height;
uint32_t width;

/* Properties of the narrow horizontal strip, with crosses */
uint32_t narrowheight;
uint32_t gapwidth;
uint32_t narrowwidth; /* Useful width */
uint32_t narrowpixels; /* Useful pixels */

uint32_t wideheight;
uint32_t widewidth;
uint32_t widepixels;

uint32_t repheight;
uint32_t reppixels;

uint64_t totalbits;

uint32_t fec_syms;
uint32_t netbits;
uint32_t usedbits;

uint32_t border = 2;
uint32_t chalf = 3;
uint32_t cpitch = 24;
uint32_t xcrosses = 32;
uint32_t ycrosses = 46;
uint32_t text_width = 13; // has to be 13!! Do not change!

/* initialize rest of the values based on xcrosses and ycrosses */
void init_values(uint32_t xcrosses_input, uint32_t ycrosses_input) {
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
	totalbits = (uint64_t)reppixels*(ycrosses-1)+narrowpixels;
	fec_syms = totalbits/FEC_LARGEBITS;
	netbits = fec_syms*FEC_SMALLBITS;
	usedbits = fec_syms*FEC_LARGEBITS;

}


/* Coordinates don't count with the border - 0,0 is upper left corner of the
 * first cross! */
int is_cross(uint32_t x, uint32_t y)
{
	x%=cpitch;
	y%=cpitch;
	return (x<2*chalf&&y<2*chalf);
}

/* Returns the coords relative to the upperloeftmost cross upper left corner
 * pixel! If you have borders, you have to add them! */
void seq2xy(int *x, int *y, uint32_t seq)
{
	uint32_t rep; /* Repetition - number of narrow strip - wide strip pair,
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
		uint32_t gap; /* Horizontal gap number */
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
uint64_t golay(uint64_t in)
{
	return golay_codes[in&4095];
}


