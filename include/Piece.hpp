#pragma once

#include "Constants.hpp"

#include <array>
#include <iostream>
#include <cstdint>
#include <vector>

class Piece
{
	private:
		PieceType type;
		int rotationState;
		int x;
		int y;
	public:
		Piece(const PieceType& type);
		void reassign(const PieceType& type);
		void reassign(int pieceNum);
		void setNoExist();
		bool exist();
		int typeConverter(const PieceType& type) const;
		void drop();
		void undrop();
		void moveLeft();
		void moveRight();
		std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> getCoordinates() const;
		std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> getCoordinates(int kickNum, int direction, int initialRotationState) const;
		void printInfo();
		void rotateCW();
		void rotateCCW();
		void applyKick(int kickNum, int direction, int initialRotationState);
		int getRotationState();
		uint16_t getKey();
		void applyKey(uint16_t key);
		void getFeatureVector(std::vector<int>& toGet);
		PieceType getType();
};