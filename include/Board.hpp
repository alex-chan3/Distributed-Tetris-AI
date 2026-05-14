#pragma once

#include "Constants.hpp"
#include "Spawner.hpp"
#include "Piece.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>

class Board
{
	private:
		uint16_t gameBoard[ACTUAL_HEIGHT];
		int score;
		
		void addBoard(std::string& boardString);
		int getTotalHeight(std::array<int, WIDTH>& heights);
		int getMaxHeight(std::array<int, WIDTH>& heights);
		int getHoles(std::array<int, WIDTH>& heights);
		int getSmoothness(std::array<int, WIDTH>& heights);
		int getLineClears();
		void getHeights(std::array<int, WIDTH>& heights);
		void columnHoles(std::vector<int>& features, std::array<int, WIDTH>& heights);
	public:
		void initialize();
		void printGame();
		void clearLines();
		void drawPiece(const Piece& p);
		void removePiece(const Piece& p);
		bool movePieceDown(Piece& p);
		bool movePieceLeft(Piece& p);
		bool movePieceRight(Piece& p);
		bool rotateCW(Piece& p);
		bool rotateCCW(Piece& p);
		bool checkLose();
		bool collisionCheck(Piece& p);
		int kickCollisionCheck(Piece& p, int direction, int initialRotationState);
		void getFeatures(std::vector<int>& results);
		void copyBoard(std::vector<uint16_t>& copyTo);
		void applyBoard(std::vector<uint16_t>& copyFrom);
		void getBoardAsFeature(std::vector<int>& copyTo);
		void printBlocks();
};