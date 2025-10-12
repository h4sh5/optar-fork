/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */

#include <stdio.h> /* getchar */
#include <stdlib.h> /* exit */
#include <string.h> /* memcpy */
#include <assert.h> /* assert */

#define width font_width
#define height font_height
#include "font.h"
#undef width
#undef height

#include "optar.h"
#include "parity.h"
static unsigned height;
static unsigned text_height = 24;

// width is in optar.h
static unsigned char *ary; //ptr to a 2d array 
static unsigned ary_size;
static unsigned char *file_label=(unsigned char *)""; /* The filename written in the
					file_label */
static char *output_filename; /* The output filename */
static unsigned output_filename_buffer_size;
static unsigned char *base=(unsigned char *)"optar_out"; /* Output filename base */
static unsigned file_number;
FILE *output_stream;
FILE *input_stream;
unsigned n_pages; /* Number of pages calculated from the file length */

void dump_ary(void)
{
	fprintf(output_stream,
		"P5\n"
		"%u %u\n"
		"255\n"
		,width, height);

	fwrite(ary, ary_size, 1, output_stream);
}

/* Only the LSB is significant. Writes hamming-encoded bits. The sequence number
 * must not be out of range! */
void write_channelbit(unsigned char bit, unsigned long seq)
{
	int x,y; /* Positions of the pixel */

	bit&=1;
	bit=-bit;
	bit=~bit; /* White=bit 0, black=bit 1 */
	seq2xy(&x, &y, seq); /* Returns without borders! */
	x+=border;
	y+=border;
	// *ary[x+y*width]=bit;
	ary[x+y*width]=bit;
	seq++;
}

/* Groups into two groups of bits, 0...bit-1 and bit..., and then makes
 * a gap with zero between them by shifting the higer bits up. */
unsigned long split(unsigned long in, unsigned bit)
{
	unsigned long high;

	high=in;
	in&=(1UL<<bit)-1;
	high^=in;
	return (high<<1)|in;
}

/* Thie bits are always stored in the LSB side of the register. Only the
 * lowest FEC_SMALLBITS are taken into account on input. */
unsigned long hamming(unsigned long in)
{
	in&=(1UL<<FEC_SMALLBITS)-1;

	in<<=3; /* Split 0,1,2 */
#if FEC_ORDER>=3
	in=split(in,4);
#if FEC_ORDER>=4
	in=split(in,8);
#if FEC_ORDER>=5
	in=split(in,16);
	in|=parity(in&0xffff0000)<<16;
#endif
	in|=parity(in&0xff00ff00)<<8;
#endif
	in|=parity(in&0xf0f0f0f0)<<4;
#endif
	in|=parity(in&0xcccccccc)<<2;
	in|=parity(in&0xaaaaaaaa)<<1;
	in|=parity(in);
	return in;
}

void do_border(void)
{
	unsigned c;
	unsigned char *ptr = ary;

	memset(ptr,0,border*width);
	// fprintf(stderr, "[do_border] done 1st memset..\n");
	ptr += border*width;
	for (c=data_height;c;c--){
		memset(ptr,0,border);
		ptr+=width;
		memset(ptr-border,0,border);
	}
	memset(ptr,0,text_height*width);
	ptr+=text_height*width;
	/* border bytes into the bottom border */
	memset(ptr,0,border*width);
}

void do_cross(unsigned int x, unsigned int y)
{
	unsigned char *ptr = ary + y*width + x;
	unsigned c;

	for (c=chalf;c!=0;c--,ptr+=width){
		memset(ptr,0,chalf);
		memset(ptr+chalf,0xff,chalf);
		memset(ptr+chalf*width,0xff,chalf);
		memset(ptr+chalf*(width+1),0,chalf);
		
	}
}

void crosses(void)
{
	unsigned x,y;

	for (y=border;y<=height-text_height-border-2*chalf;y+=cpitch)
		for (x=border;x<=width-border-2*chalf;x+=cpitch) {
			// fprintf(stderr, "[crosses] x:%d y:%d\n", x,y);
			do_cross(x,y);

		}
}

/* x is in the range 0 to data_width-1 */
void text_block (unsigned int destx,  unsigned int srcx, unsigned int width)
{
	int x, y;
	unsigned char *srcptr;
	unsigned char *destptr;

	if (destx+width>data_width) return; /* Letter doesn't fit */

	srcptr=(unsigned char *)(void *)header_data+srcx;
	destptr= ary+ width*(border+data_height)+border+destx;

	for (y=0;y<text_height;y++, srcptr+=font_width, destptr+=width){
		for (x=0;x<width;x++){
			destptr[x]=header_data_cmap[srcptr[x]][0]&0x80?0xff:0;
		}
	}
}

void label(void)
{
	unsigned x=0;
	// char *txt = malloc(data_width/text_width);
	char txt[data_width/text_width];
	unsigned char *ptr;
	unsigned txtlen;

	snprintf(txt, sizeof txt, "  0-%u-%u-%u-%u-%u-%u-%u %u/%u %s"
		, xcrosses, ycrosses, cpitch, chalf
		, FEC_ORDER, border, text_height
		,file_number,n_pages
		, (char *)(void *)file_label);
	txtlen=strlen((char *)(void *)txt);

	assert(font_height==text_height);
	x=font_width-text_width*(127-' ');
	text_block(0,text_width*(127-' '), x);
	for (ptr=(unsigned char *)(void *)txt
			;ptr<(unsigned char *)(void *)txt+txtlen;ptr++){
		if (*ptr>=' '&&*ptr<=127){
			text_block(x,text_width*(*ptr-' '), text_width);
			x+=text_width;
		}
	}

	// free(txt);

}

void format_ary(void)
{

	memset(ary, 0xff, ary_size); /* White */
	// fprintf(stderr,"format_ary: done memset\n");
	do_border();
	// fprintf(stderr,"format_ary: done do_border\n");
	crosses();
	// fprintf(stderr,"format_ary: done crosses\n");
	label();
	// fprintf(stderr,"format_ary: done label\n");
}

/* Always formats ary. Dumps it if it's not the first one. */
void new_file(void)
{
	if (file_number){
		dump_ary();
		fclose(output_stream);
	}
	if (file_number>=9999){
		fprintf(stderr,"optar: too many pages - 10,000 or more\n");
		exit(1);
	}
	snprintf(output_filename,output_filename_buffer_size
		,"%s_%04u.pgm",(char *)(void *)base,++file_number);
	output_stream=fopen(output_filename,"w");
	if (!output_stream){
		fprintf(stderr,"optar: cannot open %s for writing.\n", output_filename);
		exit(1);
	}
	format_ary();
}

/* That's the net channel capacity */
void write_payloadbit(unsigned char bit)
{
	static unsigned long accu=1;
	static unsigned long hamming_symbol;

	accu<<=1;
	accu|=bit&1;
	if (accu&(1UL<<FEC_SMALLBITS)){
		/* Full payload */
		int shift;

		/* Expands from FEC_SMALLBITS bits to FEC_LARGEBITS */
#if FEC_ORDER == 1
		accu=golay(accu);
#else
		accu=hamming(accu);
#endif /* FEC_ORDER */

		if (hamming_symbol>=fec_syms){
			/* We couldn't write into the page, we need to make
			 * another one */
			new_file();
			hamming_symbol=0;
		}

		/* Write the symbol into the page */
		for (shift=FEC_LARGEBITS-1;shift>=0;shift--)
			write_channelbit(accu>>shift
				, hamming_symbol+(FEC_LARGEBITS-1-shift)
				*fec_syms);
		accu=1;
		hamming_symbol++;
	}
}

void write_byte(unsigned char c)
{
	int bit;

	for (bit=7; bit>=0;bit--)
		write_payloadbit(c>>bit);
}

/* Prints the text at the bottom */
/* Makes one output file. */
void feed_data(void)
{
	int c;

	while((c=fgetc(input_stream))!=EOF){
		write_byte(c);
	}

	/* Flush the FEC with zeroes */
	for (c=FEC_SMALLBITS-1;c;c--){
		write_payloadbit(0);
	}

	dump_ary();
	fclose(output_stream);

}

void open_input_file(char *fname)
{
	input_stream=fopen(fname,"r");
	if (!input_stream){
		fprintf(stderr,"optar: cannot open input file %s: "
			,fname);
		perror("");
		exit(1);
	}
	if (fseek(input_stream, 0, SEEK_END)){
		fprintf(stderr,"optar: cannot seek to the end of %s: "
			,fname);
		perror("");
		exit(1);
	}
	n_pages=(((unsigned long)ftell(input_stream)<<3)+netbits-1)
		/netbits;
	if (fseek(input_stream,0, SEEK_SET)){
		fprintf(stderr,"optar: cannot seek to the beginning of %s: "
			,fname);
		perror("");
		exit(1);
	}
}

/* argv format:
 * 1st arg - input file
 * 2nd arg(optional) - label and output filename base */
int main(int argc, char **argv)
{

	if (argc<2){
		fprintf(stderr,
"\n"
"Usage: optar <input file> [filename base] [xcrosses] [ycrosses]\n"
"\n"
"xcrosses and ycrosses have a default value of 32 x 46, increase them to encode more data on one page but sacrifices accuracy\n"
"Will take the input file as data payload and produce optar_out_????.pgm which contain the input file encoded onto paper, with error correction codes, and automatically split into multiple files when necessary. Those pgm's are supposed to be printed on laser printer at least 600 DPI for example using GIMP, or use the included pgm2ps to  convert them to PostScript and print for example using a PostScript viewer program.\n"
"\n"
);
		exit(1);
	}

	if (argc >= 4) {
		xcrosses = atoi(argv[3]);
		ycrosses = atoi(argv[4]);
		fprintf(stderr, "taking arguments xcrosses=%d and ycrosses=%d\n", xcrosses, ycrosses);
	}

	fprintf(stderr,"initializing dimensions values using xcrosses=%d ycrosses=%d..\n", xcrosses, ycrosses);
	init_values(xcrosses, ycrosses);
	height= (2*border+data_height+text_height);
	fprintf(stderr, "done initializing values..\n");
	fprintf(stderr, "width:%d height:%d\n",width,height);
	ary = malloc(width*height);
	ary_size = width*height;
	fprintf(stderr, "doing open_input_file\n");
	open_input_file(argv[1]);
	fprintf(stderr, "done open_input_file\n");

	if (argc>=3) file_label=base=(void *)argv[2];
	fprintf(stderr, "file_label: %s\n", file_label);
	output_filename_buffer_size=strlen((char *)(void *)file_label)+1+4+1+3+1;
	output_filename=malloc(output_filename_buffer_size);
	if (!output_filename){
		fprintf(stderr,"Cannot allocate output filename\n");
		exit(1);
	}
	fprintf(stderr, "doing new_file\n");
	new_file();
	fprintf(stderr, "done new_file\n");

	fprintf(stderr, "doing feed_data\n");
	feed_data();
	fprintf(stderr, "done feed_data\n");

	fclose(input_stream);

	free(output_filename);
	free(ary);
	return 0;
}
