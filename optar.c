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
		,g_width, height);

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
	x+=g_border;
	y+=g_border;
	// *ary[x+y*width]=bit;
	ary[x+y*g_width]=bit;
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

	memset(ptr,0,g_border*g_width);
	// fprintf(stderr, "[do_border] done 1st memset..\n");
	ptr += g_border*g_width;
	for (c=g_data_height;c;c--){
		memset(ptr,0,g_border);
		ptr+=g_width;
		memset(ptr-g_border,0,g_border);
	}
	memset(ptr,0,text_height*g_width);
	ptr+=text_height*g_width;
	/* border bytes into the bottom border */
	memset(ptr,0,g_border*g_width);
}

void do_cross(unsigned int x, unsigned int y)
{
	unsigned char *ptr = ary + y*g_width + x;
	unsigned c;

	for (c=g_chalf;c!=0;c--,ptr+=g_width){
		memset(ptr,0,g_chalf);
		memset(ptr+g_chalf,0xff,g_chalf);
		memset(ptr+g_chalf*g_width,0xff,g_chalf);
		memset(ptr+g_chalf*(g_width+1),0,g_chalf);
		
	}
}

void crosses(void)
{
	unsigned x,y;

	for (y=g_border;y<=height-text_height-g_border-2*g_chalf;y+=g_cpitch)
		for (x=g_border;x<=g_width-g_border-2*g_chalf;x+=g_cpitch) {
			// fprintf(stderr, "[crosses] x:%d y:%d\n", x,y);
			do_cross(x,y);

		}
}

/* x is in the range 0 to g_data_width-1 */
void text_block (unsigned int destx,  unsigned int srcx, unsigned int txt_width)
{
	int x, y;
	unsigned char *srcptr;
	unsigned char *destptr;

	if (destx+txt_width>g_data_width) {
		fprintf(stderr, "letter doesn't fit (destx:%u srcx:%u txt_width:%d)\n",destx,srcx,txt_width);
		return; /* Letter doesn't fit */
	}

	srcptr=(unsigned char *)(void *)header_data+srcx;
	destptr= ary + g_width*(g_border+g_data_height)+g_border+destx;

	for (y=0;y<text_height;y++, srcptr+=font_width, destptr+=g_width){
		for (x=0;x<txt_width;x++){
			destptr[x]=header_data_cmap[srcptr[x]][0]&0x80?0xff:0;
		}
	}
}

void label()
{
	unsigned x=0;
	char txt[g_data_width/g_text_width];
	int txtsz = sizeof txt;
	unsigned char *ptr;
	unsigned txtlen;
	// fprintf(stderr, "font_width:%d font_height:%d g_text_width:%d\n",font_width,font_height, g_text_width);
	snprintf(txt, txtsz, "  0-%u-%u-%u-%u-%u-%u-%u %u/%u %s"
		, g_xcrosses, g_ycrosses, g_cpitch, g_chalf
		, FEC_ORDER, g_border, text_height
		,file_number,n_pages
		, file_label);
	txtlen=strlen((char *)(void *)txt);
	// fprintf(stderr, "txt:%s strlen:%d\n",txt,txtlen);

	assert(font_height==text_height);
	x=font_width - g_text_width * (127-' ');
	text_block(0,g_text_width * (127-' '), x);
	for (ptr=(unsigned char *)(void *)txt
			;ptr<(unsigned char *)(void *)txt+txtlen;ptr++){
		if (*ptr>=' ' && *ptr<=127){ // ascii printable range
			text_block(x,g_text_width*(*ptr-' '), g_text_width);
			x+=g_text_width;
		}
	}
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

		if (hamming_symbol>=g_fec_syms){
			/* We couldn't write into the page, we need to make
			 * another one */
			new_file();
			hamming_symbol=0;
		}

		/* Write the symbol into the page */
		for (shift=FEC_LARGEBITS-1;shift>=0;shift--)
			write_channelbit(accu>>shift
				, hamming_symbol+(FEC_LARGEBITS-1-shift)
				*g_fec_syms);
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
	n_pages=(((unsigned long)ftell(input_stream)<<3)+g_netbits-1)/g_netbits;
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
"Usage: optar <input file> [filename base] [xcrosses] [g_ycrosses]\n"
"\n"
"xcrosses and ycrosses have a default value of 32 x 46, increase them to encode more data on one page but sacrifices accuracy\n"
"Will take the input file as data payload and produce optar_out_????.pgm which contain the input file encoded onto paper, with error correction codes, and automatically split into multiple files when necessary. Those pgm's are supposed to be printed on laser printer at least 600 DPI for example using GIMP, or use the included pgm2ps to  convert them to PostScript and print for example using a PostScript viewer program.\n"
"\n"
);
		exit(1);
	}

	if (argc >= 4) {
		g_xcrosses = atoi(argv[3]);
		g_ycrosses = atoi(argv[4]);
		fprintf(stderr, "taking arguments xcrosses=%d and ycrosses=%d\n", g_xcrosses, g_ycrosses);
	}

	fprintf(stderr,"initializing dimensions values using xcrosses=%d ycrosses=%d..\n", g_xcrosses, g_ycrosses);
	init_values(g_xcrosses, g_ycrosses);
	height= (2*g_border+g_data_height+text_height);
	fprintf(stderr, "done initializing values..\n");
	fprintf(stderr, "width:%d height:%d totalbits:%ld netbits:%d\n",g_width,height,g_totalbits,g_netbits);
	ary = malloc(g_width*height);
	ary_size = g_width*height;
	open_input_file(argv[1]);

	if (argc>=3) file_label=base=(void *)argv[2];
	// fprintf(stderr, "file_label: %s\n", file_label);
	output_filename_buffer_size=strlen((char *)(void *)file_label)+1+4+1+3+1;
	output_filename=malloc(output_filename_buffer_size);
	if (!output_filename){
		fprintf(stderr,"Cannot allocate output filename\n");
		exit(1);
	}
	new_file();
	feed_data();
	printf("format: 0-%u-%u-%u-%u-%u-%u-%u\n", g_xcrosses, g_ycrosses,g_cpitch, g_chalf, FEC_ORDER, g_border, text_height);

	fclose(input_stream);

	free(output_filename);
	free(ary);
	return 0;
}
