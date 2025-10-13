/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */
#include <stdint.h>
uint64_t parity(uint64_t in)
{
	in^=in>>16;
	in^=in>>8;
	in^=in>>4;
	in^=in>>2;
	in^=in>>1;
	return in&1;
}

/* Counts number of '1' bits */
uint32_t ones(uint64_t in)
{
	in-=((in>>1)&0x55555555UL); /* 2-bit groups result with max. 10 */
	in=(in&0x33333333UL)+((in&0xccccccccUL)>>2); /* 4-bit groups with 
							max. 100 */
	in+=in>>4;
	in&=0x0f0f0f0fUL; /* 8-bit groups with max. 1000 */
	in+=in>>8;
	in+=in>>16;
	return in&0x3f;
}


