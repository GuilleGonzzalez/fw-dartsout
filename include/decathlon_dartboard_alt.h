#ifndef __DECATHLON_DARTBOARD_ALT_H
#define __DECATHLON_DARTBOARD_ALT_H

#include "dartboard.h"

#define N_ROWS   7
#define N_COLS   10
#define ROWS_OFF 2 // First row pin
#define COLS_OFF 0 // First col pin

static int map_numbers[N_ROWS][N_COLS] = {
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, BULL, BULL},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
};

static int map_zones[N_ROWS][N_COLS] = {
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, SINGLE_EXT, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
};

#endif // __DECATHLON_DARTBOARD_ALT_H