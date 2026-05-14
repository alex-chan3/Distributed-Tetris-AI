#pragma once

#include "Constants.hpp"
#include "Board.hpp"

#include <iostream>
#include <conio.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <bitset>
#include <chrono>

#include <windows.h>

class Game
{
	private:
		std::vector<int> results;
		std::vector<int> bag;
		std::vector<int> bag2;
		std::vector<std::vector<uint16_t>> boardsForAI;
		int pieceCounter;
		Board b;
		Piece p;
		Piece p_held;
		size_t swapBorder;
		std::vector<int> features;
		std::vector<std::vector<int>> gameFeatures;
		int linesCleared;
	public:
		Game();
		int humanMain();
		void BFSHelper(std::vector<int>& results, const std::vector<int>& pieceData, Piece p_sim);
		void BFSHelper(std::vector<int>& results, Piece p_sim);
		std::vector<int>& getNextSteps();
		float getScore(const std::vector<int>& prev, const std::vector<int>& curr);
		float applyBoard(int boardNum);
		bool gameLost();
		int getLinesCleared();
};