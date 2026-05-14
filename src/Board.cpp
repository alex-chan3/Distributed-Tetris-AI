#include "Board.hpp"

void Board::initialize()
{
	for (int i = 0; i < ACTUAL_HEIGHT; i++) {
		gameBoard[i] = 0;
	}
	
	score = 0;
}

void Board::addBoard(std::string& boardString)
{
	for (int i = 2; i < ACTUAL_HEIGHT; i++) {
		boardString.append("|");
		for (int j = 0; j < WIDTH; j++) {
			if (((gameBoard[i] >> j) & 1) == 0) {
				boardString.append("   ");
			}
			else {
				boardString.append("[ ]");
			}
		}
		boardString.append("|");
		boardString.append("\n");
	}
	boardString.append("================================");
}

void Board::printGame()
{
	std::string toPrint;
	toPrint.reserve(DEFAULT_SIZE);
	toPrint.append("Board:\n\n");
	addBoard(toPrint);
	
	//toPrint.append("\n\nScore: ");
	//toPrint.append(std::to_string(score));
	//toPrint.append("\n");
	
	//std::cout << "\033[2J\033[1;1H";
	std::cout << toPrint << std::flush;
}

void Board::clearLines()
{
	int currRow = ACTUAL_HEIGHT - 1;
	uint16_t tempBoard[ACTUAL_HEIGHT] = {0};
	
	for (int h = ACTUAL_HEIGHT - 1; h >= 0; h--) {
		bool full = true;
		
		full = (gameBoard[h] == FULL_ROW);
		
		if (!full) {
			tempBoard[currRow] = gameBoard[h];
			currRow--;
		}
	}
	
	for (int i = 0; i < ACTUAL_HEIGHT; i++) {
		gameBoard[i] = tempBoard[i];
	}
}

void Board::drawPiece(const Piece& p)
{
	std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates = p.getCoordinates();
	
	for (int i = 0; i < NUM_COORDINATES; i++) {
		gameBoard[coordinates[i][1]] |= (1 << coordinates[i][0]);
	}
}

void Board::removePiece(const Piece& p)
{
	std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates = p.getCoordinates();
	
	for (int i = 0; i < NUM_COORDINATES; i++) {
		gameBoard[coordinates[i][1]] &= ~(1 << coordinates[i][0]);
	}
}

bool Board::collisionCheck(Piece& p)
{
	std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates = p.getCoordinates();
	
	for (int i = 0; i < NUM_COORDINATES; i++) {
		if (coordinates[i][0] < 0 || coordinates[i][0] >= WIDTH) {
			return true;
		}
		else if (coordinates[i][1] < 0 || coordinates[i][1] >= ACTUAL_HEIGHT) {
			return true;
		}
		else if (((gameBoard[coordinates[i][1]] >> coordinates[i][0]) & 1) != 0) {
			return true;
		}
	}
	
	return false;
}

int Board::kickCollisionCheck(Piece& p, int direction, int initialRotationState)
{
	bool match = true;
	for (int i = 0; i < SRS_SIZE; i++) {
		match = true;
		std::array<std::array<int, COORDINATES_SIZE>, NUM_COORDINATES> coordinates = p.getCoordinates(i, direction, initialRotationState);
		for (int j = 0; j < NUM_COORDINATES; j++) {
			if (coordinates[j][0] < 0 || coordinates[j][0] >= WIDTH) {
				match = false;
				break;
			}
			else if (coordinates[j][1] < 0 || coordinates[j][1] >= ACTUAL_HEIGHT) {
				match = false;
				break;
			}
			else if (((gameBoard[coordinates[j][1]] >> coordinates[j][0]) & 1) != 0) {
				match = false;
				break;
			}
		}
		
		if (match) {
			return i;
		}
	}
	
	return -1;
}

bool Board::movePieceDown(Piece& p)
{
	removePiece(p);
	p.drop();
	int num = 2;
	
	if (!collisionCheck(p)) {
		num = 2;
	}
	else {
		num = 1;
		p.undrop();
	}
	
	drawPiece(p);
	
	return num == 2;
}

bool Board::movePieceLeft(Piece& p)
{
	removePiece(p);
	p.moveLeft();
	int num = 2;
	
	if (!collisionCheck(p)) {
		num = 2;
	}
	else {
		num = 1;
		p.moveRight();
	}
	
	drawPiece(p);
	
	return num == 2;
}

bool Board::movePieceRight(Piece& p)
{
	removePiece(p);
	p.moveRight();
	int num = 2;
	
	if (!collisionCheck(p)) {
		num = 2;
	}
	else {
		num = 1;
		p.moveLeft();
	}
	
	drawPiece(p);
	
	return num == 2;
}

bool Board::rotateCW(Piece& p)
{
	removePiece(p);
	int num = 2;
	int initialRotationState = p.getRotationState();
	
	p.rotateCW();
	int kickResult = kickCollisionCheck(p, CW, initialRotationState);
	//std::cout << "rotation state: " << initialRotationState << " kick result: " << kickResult << "\n";
	if (kickResult != -1) {
		p.applyKick(kickResult, CW, initialRotationState);
		num = 2;
	}
	else {
		p.rotateCCW();
		num = 1;
	}
	
	drawPiece(p);
	
	return num == 2;
}

bool Board::rotateCCW(Piece& p)
{
	removePiece(p);
	int num = 2;
	int initialRotationState = p.getRotationState();
	
	p.rotateCCW();
	int kickResult = kickCollisionCheck(p, CCW, initialRotationState);
	//std::cout << "rotation state: " << initialRotationState << " kick result: " << kickResult << "\n";
	if (kickResult != -1) {
		p.applyKick(kickResult, CCW, initialRotationState);
		num = 2;
	}
	else {
		p.rotateCW();
		num = 1;
	}
	
	drawPiece(p);
	
	return num == 2;
}

bool Board::checkLose()
{
	for (int i = 0; i < 2; i++) {
		if (gameBoard[i] != 0) {
			return true;
		}
	}
	return false;
}

int Board::getTotalHeight(std::array<int, WIDTH>& heights)
{
	int result = 0;
	
	for (int c = 0; c < WIDTH; c++) {
		result += heights[c];
	}
	
	return result;
}

int Board::getMaxHeight(std::array<int, WIDTH>& heights)
{
	int result = 0;
	
	for (int c = 0; c < WIDTH; c++) {
		if (heights[c] > result) {
			result = heights[c];
		}
	}
	
	return result;
}

int Board::getHoles(std::array<int, WIDTH>& heights)
{
	int result = 0;
	
	for (int i = 0; i < WIDTH; i++) {		
		for (int h = ACTUAL_HEIGHT - heights[i] + 1; h < ACTUAL_HEIGHT; h++) {
			if (((gameBoard[h] >> i) & 1) == 0) {
				result++;
			}
		}
	}
	
	return result;
}

int Board::getSmoothness(std::array<int, WIDTH>& heights)
{
	int result = 0;
	
	for (int i = 0; i < WIDTH - 1; i++) {
		if (heights[i] > heights[i + 1]) {
			result += heights[i] - heights[i + 1];
		}
		else {
			result += heights[i + 1] - heights[i];
		}
	}
	
	return result;
}

int Board::getLineClears()
{
	int result = 0;
	
	for (int h = 0; h < ACTUAL_HEIGHT; h++) {
		if (gameBoard[h] == FULL_ROW) {
			result++;
		}
	}
	
	return result;
}

void Board::getHeights(std::array<int, WIDTH>& heights)
{
	int h;
	
	for (int c = 0; c < WIDTH; c++) {
		h = 0;
		while (h < ACTUAL_HEIGHT && ((gameBoard[h] >> c) & 1) == 0) {
			h++;
		}
		heights[c] = ACTUAL_HEIGHT - h;
	}
}

void Board::columnHoles(std::vector<int>& features, std::array<int, WIDTH>& heights)
{
	int counter = 0;
	
	for (int i = 0; i < WIDTH; i++) {
		counter = 0;
		for (int h = ACTUAL_HEIGHT - heights[i] + 1; h < ACTUAL_HEIGHT; h++) {
			if (((gameBoard[h] >> i) & 1) == 0) {
				counter++;
			}
		}
		features.push_back(counter);
	}
}

void Board::getFeatures(std::vector<int>& results)
{
	uint16_t gameBoardCopy[ACTUAL_HEIGHT];
	
	std::array<int, WIDTH> heights;
	
	std::memcpy(
		gameBoardCopy,
		gameBoard,
		ACTUAL_HEIGHT * sizeof(uint16_t)
	);
	
	int linesCleared = getLineClears();
	clearLines();
	
	getHeights(heights);
	
	/*std::vector<int> results = {
		linesCleared,
		getHoles(heights),
		getTotalHeight(heights),
		getMaxHeight(heights),
		getSmoothness(heights)
		};
	*/
	
	results.clear();
	results.reserve(23);
	
	results.push_back(linesCleared);
	
	for (int height : heights) {
		results.push_back(height);
	}
	
	columnHoles(results, heights);
	
	results.push_back(getSmoothness(heights));
	results.push_back(getMaxHeight(heights));
	
	std::memcpy(
		gameBoard,
		gameBoardCopy,
		ACTUAL_HEIGHT * sizeof(uint16_t)
	);
}

void Board::copyBoard(std::vector<uint16_t>& copyTo)
{
	copyTo.resize(ACTUAL_HEIGHT);
	
	std::memcpy(
		copyTo.data(),
		gameBoard,
		ACTUAL_HEIGHT * sizeof(uint16_t)
	);
}

void Board::applyBoard(std::vector<uint16_t>& copyFrom)
{
	std::memcpy(
		gameBoard,
		copyFrom.data(),
		ACTUAL_HEIGHT * sizeof(uint16_t)
	);
}

void Board::getBoardAsFeature(std::vector<int>& copyTo)
{
	for (int i = 2; i < ACTUAL_HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			copyTo.push_back((gameBoard[i] >> j) & 1);
		}
	}
}

void Board::printBlocks()
{
	int counter = 0;
	
	for (int i = 0; i < ACTUAL_HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			if (((gameBoard[i] >> j) & 1) == 1)
				counter++;
		}
	}
	
	std::cout << "Blocks: " << counter << "\n";
}
