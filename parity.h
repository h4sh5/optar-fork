/* (c) GPL 2007 Karel 'Clock' Kulhavy, Twibright Labs */
#ifndef PARITY_H
#define PARITY_H

#include <stdint.h>

extern uint64_t parity(uint64_t in);

/* Counts number of '1' bits */
extern uint32_t ones(uint64_t in);

#endif