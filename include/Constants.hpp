#pragma once

#define DEFAULT_SIZE 2048
#define ACTUAL_HEIGHT 22
#define HEIGHT 20
#define WIDTH 10
#define NUM_PIECES 7
#define NUM_ROTATIONS 4
#define NUM_ROTATION_DIRECTIONS 2
#define NUM_COORDINATES 4
#define COORDINATES_SIZE 2
#define SRS_SIZE 5
#define CW 0
#define CCW 1
#define FULL_ROW 0b1111111111
#define X_SHIFT 5
#define Y_SHIFT 2
#define Y_MASK 0b11111
#define ROTATION_MASK 0b11

enum class PieceType
{
	I, O, T, S, Z, J, L
};

const int relativeCoordinates[NUM_PIECES][NUM_ROTATIONS][NUM_COORDINATES][COORDINATES_SIZE] = 
{
	//I coordinates
	{
		{{0, 0}, {1, 0}, {2, 0}, {3, 0}},
		{{2, -1}, {2, 0}, {2, 1}, {2, 2}},
		{{0, 1}, {1, 1}, {2, 1}, {3, 1}},
		{{1, -1}, {1, 0}, {1, 1}, {1, 2}}
	},
	//O coordinates
	{
		{{0, 0}, {1, 0}, {1, 1}, {0, 1}},
		{{0, 0}, {1, 0}, {1, 1}, {0, 1}},
		{{0, 0}, {1, 0}, {1, 1}, {0, 1}},
		{{0, 0}, {1, 0}, {1, 1}, {0, 1}}
	},
	//T coordinates
	{
		{{0, 0}, {0, -1}, {-1, 0}, {1, 0}},
		{{0, 0}, {1, 0}, {0, -1}, {0, 1}},
		{{0, 0}, {0, 1}, {-1, 0}, {1, 0}},
		{{0, 0}, {-1, 0}, {0, -1}, {0, 1}}
	},
	//S coordinates
	{
		{{0, 0}, {-1, 0}, {0, -1}, {1, -1}},
		{{0, 0}, {0, -1}, {1, 0}, {1, 1}},
		{{0, 0}, {1, 0}, {0, 1}, {-1, 1}},
		{{0, 0}, {0, 1}, {-1, 0}, {-1, -1}}
	},
	//Z coordinates
	{
		{{0, 0}, {-1, -1}, {0, -1}, {1, 0}},
		{{0, 0}, {1, -1}, {1, 0}, {0, 1}},
		{{0, 0}, {-1, 0}, {0, 1}, {1, 1}},
		{{0, 0}, {0, -1}, {-1, 0}, {-1, 1}}
	},
	//J coordinates
	{
		{{0, 0}, {-1, -1}, {-1, 0}, {1, 0}},
		{{0, 0}, {1, -1}, {0, -1}, {0, 1}},
		{{0, 0}, {1, 1}, {1, 0}, {-1, 0}},
		{{0, 0}, {-1, 1}, {0, -1}, {0, 1}}
	},
	//L coordinates
	{
		{{0, 0}, {-1, 0}, {1, 0}, {1, -1}},
		{{0, 0}, {0, -1}, {0, 1}, {1, 1}},
		{{0, 0}, {1, 0}, {-1, 0}, {-1, 1}},
		{{0, 0}, {0, -1}, {0, 1}, {-1, -1}}
	}
};

const int SRSKickCoordinatesNonI[NUM_ROTATIONS][SRS_SIZE][NUM_ROTATION_DIRECTIONS][COORDINATES_SIZE] = {
	//0 coordinates
	{
		{{0, 0}, {0, 0}},
		{{-1, 0}, {1, 0}},
		{{-1, 1}, {1, 1}},
		{{0, -2}, {0, -2}},
		{{-1, -2}, {1, -2}}
	},	
	//1 coordinates
	{
		{{0, 0}, {0, 0}},
		{{1, 0}, {1, 0}},
		{{1, -1}, {1, -1}},
		{{0, 2}, {0, 2}},
		{{1, 2}, {1, 2}}
	},
	//2 coordinates
	{
		{{0, 0}, {0, 0}},
		{{1, 0}, {-1, 0}},
		{{1, 1}, {-1, 1}},
		{{0, -2}, {0, -2}},
		{{1, -2}, {-1, -2}}
	},
	//3 coordinates
	{
		{{0, 0}, {0, 0}},
		{{-1, 0}, {-1, 0}},
		{{-1, -1}, {-1, -1}},
		{{0, 2}, {0, 2}},
		{{-1, 2}, {-1, 2}}
	},
};

const int SRSKickCoordinatesIBlock[NUM_ROTATIONS][SRS_SIZE][NUM_ROTATION_DIRECTIONS][COORDINATES_SIZE] = {
	//0 coordinates
	{
		{{0, 0}, {0, 0}},
		{{-2, 0}, {-1, 0}},
		{{1, 0}, {2, 0}},
		{{-2, -1}, {-1, 2}},
		{{1, 2}, {2, -1}}
	},	
	//1 coordinates
	{
		{{0, 0}, {0, 0}},
		{{1, 0}, {-2, 0}},
		{{-2, 0}, {1, 0}},
		{{1, -2}, {-2, -1}},
		{{-2, 1}, {1, 2}}
	},
	//2 coordinates
	{
		{{0, 0}, {0, 0}},
		{{2, 0}, {1, 0}},
		{{-1, 0}, {-2, 0}},
		{{2, 1}, {1, -2}},
		{{-1, -2}, {-2, 1}}
	},
	//3 coordinates
	{
		{{0, 0}, {0, 0}},
		{{-1, 0}, {2, 0}},
		{{2, 0}, {-1, 0}},
		{{-1, 2}, {2, 1}},
		{{2, -1}, {-1, -2}}
	},
};