#include "Game.hpp"

/*
todo: 
- add scoring in board.cpp, clear lines
- verify relativecoordinates
- remove windows.h before uploading, specifically getting the input
- look into checking if we need to have current piece as 2 rather than 1 (coordinates keep track)
*/

Game::Game() : bag(generatePieces()), bag2(generatePieces()), pieceCounter(0), p(static_cast<PieceType>(bag[pieceCounter])), p_held(static_cast<PieceType>(bag[pieceCounter]))
{
	b.initialize();
	pieceCounter++;
	p_held.setNoExist();
	swapBorder = 0;
	features.clear();
	gameFeatures.clear();
	linesCleared = 0;
}

int Game::humanMain()
{
	int frameCounter = 0;
	char input = ' ';
	bool done = true;
	bool down = false;
	bool left = false;
	bool right = false;
		
	while (true) {
		Sleep(100);
		frameCounter++;

		down = GetAsyncKeyState(VK_DOWN) & 0x8000;		
		left = GetAsyncKeyState(VK_LEFT) & 0x8000;
		right = GetAsyncKeyState(VK_RIGHT) & 0x8000;
		
		if (down) {
			done = b.movePieceDown(p);
			if (!done) {
				b.clearLines();
				if (pieceCounter == NUM_PIECES) {
					pieceCounter = 0;
					bag = generatePieces();
				}
				p.reassign(static_cast<PieceType>(bag[pieceCounter]));
				pieceCounter++;
				done = true;
				if (b.checkLose()) {
					b.printGame();
					break;
				}
				b.drawPiece(p);
			}
		}
		if (left) {
			b.movePieceLeft(p);
		}
		if (right) {
			b.movePieceRight(p);
		}
		if (_kbhit() && (input = _getch()) == -32) {
			input = _getch();
			if (input == 80 && !down) {
				done = b.movePieceDown(p);
				if (!done) {
					b.clearLines();
					if (pieceCounter == NUM_PIECES) {
						pieceCounter = 0;
						bag = generatePieces();
					}
					p.reassign(static_cast<PieceType>(bag[pieceCounter]));
					pieceCounter++;
					done = true;
					if (b.checkLose()) {
						b.printGame();
						break;
					}
					b.drawPiece(p);
				}
			}
			else if (input == 75 && !left) {
				b.movePieceLeft(p);
			}
			else if (input == 77 && !right) {
				b.movePieceRight(p);
			}
		}
		if (input == 'e') {
			//std::cout << "Clockwise rotation\n";
			b.rotateCW(p);
			input = ' ';
		}
		else if (input == 'q') {
			//std::cout << "Counter clockwise rotation\n";
			b.rotateCCW(p);
			input = ' ';
		}

		if (frameCounter == 20) {
			frameCounter = 0;
			if (!b.movePieceDown(p)) {
				b.clearLines();
				if (pieceCounter == NUM_PIECES) {
					pieceCounter = 0;
					bag = generatePieces();
				}
				p.reassign(static_cast<PieceType>(bag[pieceCounter]));
				pieceCounter++;
				done = true;
				if (b.checkLose()) {
					b.printGame();
					break;
				}
			}
		}
		
		b.printGame();
	}
	
	return 0;
}

void Game::BFSHelper(std::vector<int>& results, const std::vector<int>& pieceData, Piece p_sim)
{	
	std::vector<uint16_t> tempArray;
	tempArray.reserve(ACTUAL_HEIGHT);
	
	std::vector<int> tempResults;
	tempResults.reserve(221);
	std::vector<int> tempFeatures;
	tempFeatures.reserve(5);
	
	std::unordered_set<uint16_t> seen;
	std::unordered_set<uint64_t> seenPlacements;
	std::queue<uint16_t> toDo;
	
	uint16_t key = p_sim.getKey();
	uint16_t tempKey;
	seen.insert(key);
	toDo.push(key);
	
	int initialRotationState = p_sim.getRotationState();
	int kickResult;
	
	//bfs for piece being dropped
	while (!toDo.empty()) {
		size_t n = toDo.size();
		for (int i = 0; i < n; i++) {
			tempResults.clear();
			tempFeatures.clear();
			key = toDo.front();
			toDo.pop();
			p_sim.applyKey(key);
			initialRotationState = p_sim.getRotationState();
			
			p_sim.drop();
			if (!b.collisionCheck(p_sim)) {
				tempKey = p_sim.getKey();
				if (!seen.contains(tempKey)) {
					seen.insert(tempKey);
					toDo.push(tempKey);
				}
			}
			else {
				p_sim.applyKey(key);
				auto coords = p_sim.getCoordinates();
				std::array<uint64_t, 4> cellHashes;
				
				for (int j = 0; j < 4; j++) {
					cellHashes[j] = (uint16_t) (coords[j][0] | (coords[j][1] << 5));
				}
				std::sort(cellHashes.begin(), cellHashes.end());
				uint64_t finalHash = (uint64_t) cellHashes[0] | (uint64_t) cellHashes[1] << 16 | (uint64_t) cellHashes[2] << 32 | (uint64_t) cellHashes[3] << 48;
				
				if (!seenPlacements.contains(finalHash)) {
					seenPlacements.insert(finalHash);
					b.drawPiece(p_sim);
					b.getFeatures(tempFeatures);
					gameFeatures.push_back(tempFeatures);
					b.copyBoard(tempArray);
					boardsForAI.push_back(tempArray);
					tempArray.clear();
					tempArray.reserve(ACTUAL_HEIGHT);
					b.getBoardAsFeature(tempResults);
					tempResults.insert(tempResults.end(), pieceData.begin(), pieceData.end());
					b.removePiece(p_sim);
					results.insert(results.end(), tempResults.begin(), tempResults.end());
				}
			}
			p_sim.applyKey(key);
			
			p_sim.moveLeft();
			if (!b.collisionCheck(p_sim)) {
				tempKey = p_sim.getKey();
				if (!seen.contains(tempKey)) {
					seen.insert(tempKey);
					toDo.push(tempKey);
				}
			}
			p_sim.applyKey(key);
			
			p_sim.moveRight();
			if (!b.collisionCheck(p_sim)) {
				tempKey = p_sim.getKey();
				if (!seen.contains(tempKey)) {
					seen.insert(tempKey);
					toDo.push(tempKey);
				}
			}
			p_sim.applyKey(key);
			
			p_sim.rotateCW();
			kickResult = b.kickCollisionCheck(p_sim, CW, initialRotationState);
			if (kickResult != -1) {
				p_sim.applyKick(kickResult, CW, initialRotationState);
				tempKey = p_sim.getKey();
				if (!seen.contains(tempKey)) {
					seen.insert(tempKey);
					toDo.push(tempKey);
				}
			}
			p_sim.applyKey(key);

			p_sim.rotateCCW();
			kickResult = b.kickCollisionCheck(p_sim, CCW, initialRotationState);
			if (kickResult != -1) {
				p_sim.applyKick(kickResult, CCW, initialRotationState);
				tempKey = p_sim.getKey();
				if (!seen.contains(tempKey)) {
					seen.insert(tempKey);
					toDo.push(tempKey);
				}
			}
		}
	}
}

void Game::BFSHelper(std::vector<int>& results, Piece p_sim)
{	
	//auto start = std::chrono::high_resolution_clock::now();
	
	std::vector<uint16_t> tempArray(ACTUAL_HEIGHT);
	
	std::vector<int> tempFeatures;
	tempFeatures.reserve(21);
	
	std::bitset<65536> seen;
	std::unordered_set<uint64_t> seenPlacements;
	std::vector<uint16_t> toDo;
	toDo.reserve(1600);
	
	size_t currBFSIndex = 0;
	
	uint16_t key = p_sim.getKey();
	uint16_t tempKey;
	seen[key] = true;
	toDo.push_back(key);
	
	int initialRotationState = p_sim.getRotationState();
	int kickResult;
	
	//bfs for piece being dropped
	while (currBFSIndex < toDo.size()) {
		tempFeatures.clear();
		key = toDo[currBFSIndex++];
		p_sim.applyKey(key);
		initialRotationState = p_sim.getRotationState();
		
		p_sim.drop();
		if (!b.collisionCheck(p_sim)) {
			tempKey = p_sim.getKey();
			if (!seen[tempKey]) {
				seen[tempKey] = true;
				toDo.push_back(tempKey);
			}
		}
		else {
			p_sim.applyKey(key);
			auto coords = p_sim.getCoordinates();
			std::array<uint64_t, 4> cellHashes;
			
			for (int j = 0; j < 4; j++) {
				cellHashes[j] = (uint16_t) (coords[j][0] | (coords[j][1] << 5));
			}
			
			#define TEMP_SORT(a, b) if (a > b) std::swap(a, b)
			TEMP_SORT(cellHashes[0], cellHashes[1]);
			TEMP_SORT(cellHashes[2], cellHashes[3]);
			TEMP_SORT(cellHashes[0], cellHashes[2]);
			TEMP_SORT(cellHashes[1], cellHashes[3]);
			TEMP_SORT(cellHashes[1], cellHashes[2]);
			#undef TEMP_SORT
			uint64_t finalHash = (uint64_t) cellHashes[0] | (uint64_t) cellHashes[1] << 16 | (uint64_t) cellHashes[2] << 32 | (uint64_t) cellHashes[3] << 48;
			
			if (!seenPlacements.contains(finalHash)) {
				seenPlacements.insert(finalHash);
				b.drawPiece(p_sim);
				b.getFeatures(tempFeatures);
				gameFeatures.push_back(tempFeatures);
				b.copyBoard(tempArray);
				boardsForAI.push_back(tempArray);
				b.removePiece(p_sim);
				results.insert(results.end(), tempFeatures.begin(), tempFeatures.end());
			}
		}
		p_sim.applyKey(key);
		
		p_sim.moveLeft();
		if (!b.collisionCheck(p_sim)) {
			tempKey = p_sim.getKey();
			if (!seen[tempKey]) {
				seen[tempKey] = true;
				toDo.push_back(tempKey);
			}
		}
		p_sim.applyKey(key);
		
		p_sim.moveRight();
		if (!b.collisionCheck(p_sim)) {
			tempKey = p_sim.getKey();
			if (!seen[tempKey]) {
				seen[tempKey] = true;
				toDo.push_back(tempKey);
			}
		}
		p_sim.applyKey(key);
		
		p_sim.rotateCW();
		kickResult = b.kickCollisionCheck(p_sim, CW, initialRotationState);
		if (kickResult != -1) {
			p_sim.applyKick(kickResult, CW, initialRotationState);
			tempKey = p_sim.getKey();
			if (!seen[tempKey]) {
				seen[tempKey] = true;
				toDo.push_back(tempKey);
			}
		}
		p_sim.applyKey(key);

		p_sim.rotateCCW();
		kickResult = b.kickCollisionCheck(p_sim, CCW, initialRotationState);
		if (kickResult != -1) {
			p_sim.applyKick(kickResult, CCW, initialRotationState);
			tempKey = p_sim.getKey();
			if (!seen[tempKey]) {
				seen[tempKey] = true;
				toDo.push_back(tempKey);
			}
		}
	}
	
	//auto end = std::chrono::high_resolution_clock::now();
	//std::cout << "BFS Helper: " << std::chrono::duration<double>(end - start).count() << "s\n";
}

std::vector<int>& Game::getNextSteps()
{
	//auto start = std::chrono::high_resolution_clock::now();
	
	results.clear();
	results.reserve(20000);
	
	//std::vector<int> pieceData;
	
	boardsForAI.clear();
	boardsForAI.reserve(100);
	
	gameFeatures.clear();
	gameFeatures.reserve(100);
	
	swapBorder = 0;
	
	Piece p_next(static_cast<PieceType>(bag[pieceCounter]));
	if (pieceCounter == (NUM_PIECES - 1)) {
		p_next.reassign(bag2[0]);
	}
	else {
		p_next.reassign(bag[pieceCounter + 1]);
	}

	/*p.getFeatureVector(pieceData);
	p_next.getFeatureVector(pieceData);
	p_held.getFeatureVector(pieceData);*/

	Piece p_to_do(static_cast<PieceType>(bag[pieceCounter]));
	PieceType type = p.getType();
	p_to_do.reassign(type);

	//BFSHelper(results, pieceData, p_to_do);
	BFSHelper(results, p_to_do);

	swapBorder = boardsForAI.size();

	//pieceData.clear();
	
	if (p_held.exist()) {
		//if p_held exists, simulate hold by swapping p_held and p
		type = p_held.getType();
		//p_held.getFeatureVector(pieceData);
		//p_next.getFeatureVector(pieceData);
	}
	else {
		//if nothing is held, hold p, p_next is now being played and p_next next element will be "next piece"
		type = p_next.getType();
		/*p_next.getFeatureVector(pieceData);
		
		int nextNextCounter = pieceCounter + 1;
		Piece p_temp(static_cast<PieceType>(bag[0]));

		if (nextNextCounter >= NUM_PIECES) {
			p_temp.reassign(bag2[nextNextCounter - NUM_PIECES]);
		}
		else {
			p_temp.reassign(bag[nextNextCounter]);
		}
		
		p_temp.getFeatureVector(pieceData);*/
	}
	
	//p.getFeatureVector(pieceData);
	p_to_do.reassign(type);

	//BFSHelper(results, pieceData, p_to_do);
	BFSHelper(results, p_to_do);

	//auto end = std::chrono::high_resolution_clock::now();
	//std::cout << "Game: " << std::chrono::duration<double>(end - start).count() << "s\n";

	return results;
}

/*float Game::getScore(const std::vector<int>& prev, const std::vector<int>& curr)
{
	int lineClears = curr[0] - (prev.size() > 0 ? prev[0] : 0);
	int holeDelta = curr[1] - (prev.size() > 0 ? prev[1] : 0);
	int heightDelta = curr[2] - (prev.size() > 0 ? prev[2] : 0);
	int smoothnessDelta = curr[4] - (prev.size() > 0 ? prev[4] : 0);
	
	float reward = 0;
	if (lineClears == 1)
		reward += 10.0f;
	else if (lineClears == 2)
		reward += 30.0f;
	else if (lineClears == 3)
		reward += 60.0f;
	else if (lineClears == 4)
		reward += 120.0f;

	reward -= holeDelta * 8.0f;
	reward -= heightDelta * 0.5f;
	reward -= smoothnessDelta * 0.5f;
	
	if (b.checkLose())
		reward -= 100.0f;
	
	reward += 0.5f;
	
	return reward / 100.0;
}*/

/*
MLP lines cleared, holes, max height, smoothness reward.
float Game::getScore(const std::vector<int>& prev, const std::vector<int>& curr)
{
    int lineClears = curr[0];
    int holeDelta = curr[1] - (prev.size() > 0 ? prev[1] : 0);
    int maxHeightDelta = curr[3] - (prev.size() > 0 ? prev[3] : 0);
    int smoothnessDelta = curr[4] - (prev.size() > 0 ? prev[4] : 0);
    int maxHeight = curr[3];

    float reward = 0;

    if (lineClears == 1)
        reward += 40.0f;
    else if (lineClears == 2)
        reward += 100.0f;
    else if (lineClears == 3)
        reward += 200.0f;
    else if (lineClears == 4)
        reward += 400.0f;

    reward -= holeDelta * 8.0f;
    reward -= maxHeightDelta * 2.0f;
    reward -= smoothnessDelta * 1.0f;

    if (maxHeight > 10)
        reward -= (maxHeight - 10) * (maxHeight - 10) * 0.5f;

    reward += 1.0f;

    if (b.checkLose())
        reward -= 500.0f;

    return reward / 100.0f;
}*/

float Game::getScore(const std::vector<int>& prev, const std::vector<int>& curr)
{
    int lineClears = curr[0];

    int maxHeight = *std::max_element(curr.begin() + 1, curr.begin() + 11);
    int prevMaxHeight = prev.size() > 0 ? *std::max_element(prev.begin() + 1, prev.begin() + 11) : 0;
    int maxHeightDelta = maxHeight - prevMaxHeight;

    int totalHoles = 0, prevTotalHoles = 0;
    for (int i = 11; i < 21; i++)
		totalHoles += curr[i];
	
    if (prev.size() > 0)
        for (int i = 11; i < 21; i++)
			prevTotalHoles += prev[i];
    int holeDelta = totalHoles - prevTotalHoles;

    int smoothness = 0, prevSmoothness = 0;
    for (int i = 1; i < 10; i++)
		smoothness += abs(curr[i] - curr[i + 1]);
	
    if (prev.size() > 0)
        for (int i = 1; i < 10; i++)
			prevSmoothness += abs(prev[i] - prev[i + 1]);

    int smoothnessDelta = smoothness - prevSmoothness;

    float reward = 0;

    if (lineClears == 1)
        reward += 0.4f;
    else if (lineClears == 2)
        reward += 1.0f;
    else if (lineClears == 3)
        reward += 2.0f;
    else if (lineClears == 4)
        reward += 4.0f;
		//reward += 7.0f;

    //reward -= holeDelta * 0.15f;
	reward -= holeDelta * 0.08f;
    reward -= maxHeightDelta * 0.02f;
    reward -= smoothnessDelta * 0.01f;

    if (maxHeight > 10)
        reward -= (maxHeight - 10) * (maxHeight - 10) * 0.005f;

    reward += 0.001f;

    if (b.checkLose())
        reward -= 5.0f;
	
	return reward;
}

float Game::applyBoard(int boardNum)
{
	b.applyBoard(boardsForAI[boardNum]);
	b.clearLines();
	if (boardNum >= swapBorder) {
		if (p_held.exist()) {
			PieceType p_type = p.getType();
			PieceType p_held_type = p_held.getType();
			p.reassign(p_held_type);
			p_held.reassign(p_type);
		}
		else {
			p_held.reassign(p.getType());
		}
	}
	
	p.reassign(bag[pieceCounter]);
	pieceCounter++;
	if (pieceCounter == NUM_PIECES) {
		pieceCounter = 0;
		bag = bag2;
		bag2 = generatePieces();
	}
	
	float score = getScore(features, gameFeatures[boardNum]);
	linesCleared += gameFeatures[boardNum][0];
	features = gameFeatures[boardNum];
	//b.printBlocks();
	//std::cout << "lines cleared: " << linesCleared << "\n\n";
	return score;
}

bool Game::gameLost()
{
	return b.checkLose();
}

int Game::getLinesCleared()
{
	return linesCleared;
}
