#ifndef __CHINO_DARTBOARD_H
#define __CHINO_DARTBOARD_H

#include "dartboard.h"

#define N_ROWS   8
#define N_COLS   8
#define ROWS_OFF 2 // First row pin
#define COLS_OFF 0 // First col pin

static int map_numbers[N_ROWS][N_COLS] = {
	{N19, N7, N16, N8, N11, N14, N9, N12},
	{N19, N7, N16, N8, N11, N14, N9, N12},
	{N19, N7, N16, N8, N11, N14, N9, N12},
	{N3, N17, N2, N15, N10, N6, N13, N4},
	{N3, N17, N2, N15, N10, N6, N13, N4},
	{N3, N17, N2, N15, N10, N6, N13, N4},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, BULL, Z_INVALID},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, BULL, Z_INVALID},
};

static int map_zones[N_ROWS][N_COLS] = {
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, SINGLE_EXT, Z_INVALID},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, DOUBLE, Z_INVALID},
};

#endif // __CHINO_DARTBOARD_H