#include "Piece.hpp"

int Piece::typeConverter(const PieceType& type) const
{
	switch(type)
	{
		case PieceType::I:
			return 0;
		case PieceType::O:
			return 1;
		case PieceType::T:
			return 2;
		case PieceType::S:
			return 3;
		case PieceType::Z:
			return 4;
		case PieceType::J:
			return 5;
		case PieceType::L:
			return 6;
	}
	return -1;
}

Piece::Piece(const PieceType& type)
{
	reassign(type);
}

void Piece::reassign(const PieceType& type)
{
	this->type = type;
	rotationState = 0;
	reassign(typeConverter(type));
}

void Piece::setNoExist()
{
	x = -100;
	y = -100;
	rotationState = -100;
}

bool Piece::exist()
{
	return !(x == -100 && y == -100 && rotationState == -100);
}

void Piece::reassign(int pieceNum)
{
	this->type = static_cast<PieceType>(pieceNum);
	rotationState = 0;
	if (pieceNum == 0) {
		x = 3;
		y = 1;
	}
	if (pieceNum == 1) {
		x = 4;
		y = 0;
	}
	if (pieceNum == 2) {
		x = 4;
		y = 0;
	}
	if (pieceNum == 3) {
		x = 4;
		y = 0;
	}
	if (pieceNum == 4) {
		x = 4;
		y = 0;
	}
	if (pieceNum == 5) {
		x = 4;
		y = 0;
	}
	if (pieceNum == 6) {
		x = 4;
		y = 0;
	}
}


void Piece::printInfo()
{
	std::cout << "y: " << y << "\n";
	std::cout << "x: " << x << "\n";
	std::cout << "rotation state: " << rotationState << "\n";
	std::cout << "type: " << typeConverter(type) << "\n\n";
}

void Piece::drop()
{
	y++;
}

void Piece::undrop()
{
	y--;
}

void Piece::moveLeft()
{
	x--;
}

void Piece::moveRight()
{
	x++;
}

std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> Piece::getCoordinates(int kickNum, int direction, int initialRotationState) const
{
	std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates;
	int convertedType = typeConverter(type);
	
	for (int i = 0; i < NUM_COORDINATES; i++) {
			coordinates[i][0] = x + relativeCoordinates[convertedType][rotationState][i][0];
			coordinates[i][1] = y + relativeCoordinates[convertedType][rotationState][i][1];
			
			//std::cout << "initial x: " << coordinates[i][0] << " initial y: " << coordinates[i][1] << "\n";

			if (convertedType == 0) {
				coordinates[i][0] += SRSKickCoordinatesIBlock[initialRotationState][kickNum][direction][0];
				coordinates[i][1] -= SRSKickCoordinatesIBlock[initialRotationState][kickNum][direction][1];
			}
			else if (convertedType >= 1 && convertedType < NUM_PIECES) {
				coordinates[i][0] += SRSKickCoordinatesNonI[initialRotationState][kickNum][direction][0];
				coordinates[i][1] -= SRSKickCoordinatesNonI[initialRotationState][kickNum][direction][1];
			}
			//std::cout << "temp x: " << coordinates[i][0] << " temp y: " << coordinates[i][1] << "\n\n";
	}

	return coordinates;
}

std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> Piece::getCoordinates() const
{
	std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates;
	int convertedType = typeConverter(type);
	
	for (int i = 0; i < NUM_COORDINATES; i++) {
			coordinates[i][0] = x + relativeCoordinates[convertedType][rotationState][i][0];
			coordinates[i][1] = y + relativeCoordinates[convertedType][rotationState][i][1];
	}
	
	return coordinates;
}

void Piece::rotateCW()
{
	rotationState = (rotationState + 1) % 4;
}

void Piece::rotateCCW()
{
	rotationState = (4 + rotationState - 1) % 4;
}

void Piece::applyKick(int kickNum, int direction, int initialRotationState)
{
	int typeInt = typeConverter(type);
	
	if (typeInt == 0) {
		x += SRSKickCoordinatesIBlock[initialRotationState][kickNum][direction][0];
		y -= SRSKickCoordinatesIBlock[initialRotationState][kickNum][direction][1];
	}
	else if (typeInt >= 1 && typeInt < NUM_PIECES) {
		x += SRSKickCoordinatesNonI[initialRotationState][kickNum][direction][0];
		y -= SRSKickCoordinatesNonI[initialRotationState][kickNum][direction][1];
	}
	//std::cout << "x: " << x << " y: " << y << "\n";
}

int Piece::getRotationState()
{
	return rotationState;
}

uint16_t Piece::getKey()
{
	uint16_t key = 0;
	key |= (x + 2);
	key = key << X_SHIFT;
	key |= y;
	key = key << Y_SHIFT;
	key |= rotationState;
	
	return key;
}

void Piece::applyKey(uint16_t key)
{
	rotationState = key & ROTATION_MASK;
	key = key >> Y_SHIFT;
	y = key & Y_MASK;
	key = key >> X_SHIFT;
	x = (int) key - 2;
}

void Piece::getFeatureVector(std::vector<int>& toGet)
{
	if (exist()) {
		int num = typeConverter(type);
		
		for (int i = 0; i < NUM_PIECES; i++) {
			if (i != num)
				toGet.push_back(0);
			else
				toGet.push_back(1);
		}
	}
	else {
		for (int i = 0; i < NUM_PIECES; i++) {
			toGet.push_back(0);
		}
	}
}

PieceType Piece::getType()
{
	return type;
}
