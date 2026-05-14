#include "Spawner.hpp"

std::vector<int> generatePieces()
{
	std::vector<int> order = {0, 1, 2, 3, 4, 5, 6};
	
	std::random_device seed;
	std::mt19937 generator(seed());

	std::shuffle(order.begin(), order.end(), generator);
	
	return order;
}