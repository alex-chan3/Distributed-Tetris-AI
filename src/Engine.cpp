#include "Engine.hpp"

Engine::Engine() {}

void Engine::initialize(int numGames)
{
	games.clear();
	games.reserve(numGames);
	
	for (int i = 0; i < numGames; i++) {
		games.emplace_back(Game());
	}
}

std::vector<int>& Engine::getNextStep()
{
	/*for (auto& g : games) {
		results = g.getNextSteps();
	}
	
	return results;*/
	//auto start = std::chrono::high_resolution_clock::now();

	return games[0].getNextSteps();

	//auto end = std::chrono::high_resolution_clock::now();
	//std::cout << "Engine: " << std::chrono::duration<double>(end - start).count() << "s\n";
}

//change to array when we use multiple boards
float Engine::applyMove(int stepNum)
{
	/*for (int i = 0; i < games.size(); i++) {
		games[i].applyBoard(stepNum);
	}*/
	
	return games[0].applyBoard(stepNum);
}

//change to array index when using multiple boards
bool Engine::gameLost()
{
	return games[0].gameLost();
}

void Engine::helloWorld()
{
	std::cout << "Hello World\n";
}

int Engine::getLinesCleared()
{
	return games[0].getLinesCleared();
}
