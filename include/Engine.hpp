#pragma once

#include "Game.hpp"

#include <iostream>
#include <vector>
#include <utility>
#include <chrono>

class Engine
{
	private:
		std::vector<Game> games;
		std::vector<int> results;
	public:
		Engine();
		void initialize(int numGames);
		std::vector<int>& getNextStep();
		float applyMove(int stepNum);
		void helloWorld();
		bool gameLost();
		int getLinesCleared();
};