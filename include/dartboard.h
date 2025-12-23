#ifndef __DARTBOARD_H
#define __DARTBOARD_H

typedef enum {
	N_INVALID = -1,
	BULL = 0,
	N1 = 1,
	N2 = 2,
	N3 = 3,
	N4 = 4,
	N5 = 5,
	N6 = 6,
	N7 = 7,
	N8 = 8,
	N9 = 9,
	N10 = 10,
	N11 = 11,
	N12 = 12,
	N13 = 13,
	N14 = 14,
	N15 = 15,
	N16 = 16,
	N17 = 17,
	N18 = 18,
	N19 = 19,
	N20 = 20,
} numbers_t;

typedef enum {
	SINGLE_INT = 0,
	SINGLE_EXT = 1,
	DOUBLE = 2,
	TRIPLE = 3,
	Z_INVALID = 4,
} zones_t;

#endif // __DARTBOARD_H
