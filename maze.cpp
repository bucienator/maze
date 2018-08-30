// maze.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <random>
#include <iostream>
#include <fstream>

class Maze {
public:
	Maze(const unsigned width, const unsigned height)
		: width(width), height(height),
		horizontalWalls(height + 1, std::vector<bool>(width, true)),
		verticalWalls(width + 1, std::vector<bool>(height, true))
	{

	}

	const unsigned width;
	const unsigned height;
	std::vector<std::vector<bool>> horizontalWalls;
	std::vector<std::vector<bool>> verticalWalls;
};

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

void placeXX(Maze & maze, const unsigned entrySelection)
{
	if (entrySelection < maze.width) {
		// entry on top
		const unsigned entry = entrySelection;
		maze.horizontalWalls[0][entry] = false;
	}
	else if (entrySelection - maze.width < maze.height) {
		// entry on right
		const unsigned entry = entrySelection - maze.width;
		maze.verticalWalls[maze.width][entry] = false;
	}
	else if (entrySelection - maze.width - maze.height < maze.width) {
		// entry on bottom
		const unsigned entry = entrySelection - maze.width - maze.height;
		maze.horizontalWalls[maze.height][maze.width - 1 - entry] = false;
	}
	else if (entrySelection - maze.width - maze.height - maze.width < maze.height) {
		// entry on left
		const unsigned entry = entrySelection - maze.width - maze.height - maze.width;
		maze.verticalWalls[0][maze.height - 1 - entry] = false;
	}

}

void placeEntryExit(Maze & maze)
{
	const unsigned entryOptions = maze.width * 2 + maze.height * 2;
	std::uniform_int_distribution<> dis(0, entryOptions - 1);
	const unsigned entrySelection = dis(gen);

	placeXX(maze, entrySelection);

	std::uniform_int_distribution<> dis2(entrySelection + std::min(maze.width, maze.height), entrySelection + entryOptions - std::min(maze.width, maze.height));
	const unsigned exitSelection = (entrySelection + maze.width + maze.height) % entryOptions;

	placeXX(maze, exitSelection);
}

void openTheWall(std::vector<bool> & wall, const unsigned firstElement, const unsigned lastElement, const unsigned fullLength)
{
	std::uniform_int_distribution<> dis(1, fullLength);
	const unsigned isThereADoor = dis(gen) <= (lastElement - firstElement + 1);

	if (!isThereADoor)
		return;

	std::uniform_int_distribution<> dis2(firstElement, lastElement);
	const unsigned doorAt = dis2(gen);

	wall[doorAt] = false;

	if (doorAt - firstElement >= 2) {
		openTheWall(wall, firstElement, doorAt - 2, fullLength);
	}
	if (lastElement - doorAt >= 2) {
		openTheWall(wall, doorAt + 2, lastElement, fullLength);
	}
}

void generateArea(Maze & maze, const unsigned leftWall, const unsigned topWall, const unsigned rightWall, const unsigned bottomWall)
{
	const unsigned width = rightWall - leftWall;
	const unsigned height = bottomWall - topWall;

	if (width == 1 && height == 1) {
		return; // end of recursion, cannot split further
	}

	const unsigned verticalSplitOptions = width - 1;
	const unsigned horizontalSplitOptions = height - 1;
	const unsigned totalSplitOptions = verticalSplitOptions + horizontalSplitOptions;
	std::uniform_int_distribution<> dis(0, totalSplitOptions - 1);
	const unsigned splitSelection = dis(gen);

	const bool verticalSplit = splitSelection < verticalSplitOptions;

	if (verticalSplit) {
		const unsigned splitAt = splitSelection;

		openTheWall(maze.verticalWalls[leftWall + splitAt + 1], topWall, bottomWall - 1, bottomWall - topWall);

		generateArea(maze, leftWall, topWall, leftWall + splitAt + 1, bottomWall);
		generateArea(maze, leftWall + splitAt + 1, topWall, rightWall, bottomWall);
	}
	else { // horizontal split
		const unsigned splitAt = splitSelection - verticalSplitOptions;

		openTheWall(maze.horizontalWalls[topWall + splitAt + 1], leftWall, rightWall - 1, rightWall - leftWall);

		generateArea(maze, leftWall, topWall, rightWall, topWall + splitAt + 1);
		generateArea(maze, leftWall, topWall + splitAt + 1, rightWall, bottomWall);
	}

}

void printCell(const Maze & maze, const unsigned row, const unsigned col)
{
	const bool openToTop = !maze.horizontalWalls[row][col]; // bit 0
	const bool openToBottom = !maze.horizontalWalls[row+1][col]; // bit 1
	const bool openToLeft = !maze.verticalWalls[col][row]; // bit 2
	const bool openToRight = !maze.verticalWalls[col+1][row]; // bit 3
	
	const unsigned value = openToTop + openToBottom * 2 + openToLeft * 4 + openToRight * 8;

	static const char elements[] = { 'X', 'V', 'A', 186, '>', 188, 187, 185, '<', 200, 201, 204, 205, 202, 203, 206 };

	std::cout << elements[value];
}

void printMaze(const Maze & maze)
{
	for (unsigned row = 0; row < maze.height; ++row) {
		for (unsigned col = 0; col < maze.width; ++col) {
			printCell(maze, row, col);
		}
		std::cout << std::endl;
	}
}

void printMaze2(const Maze & maze)
{
	for (unsigned row = 0; row < maze.height * 2 + 1; ++row) {
		for (unsigned col = 0; col < maze.width * 2 + 1; ++col) {
			if (row % 2 == 0) {
				if (col % 2 == 0) {
					std::cout << char(219);
				}
				else {
					std::cout << (maze.horizontalWalls[row / 2][col / 2] ? char(219) : ' ');
				}
			}
			else {
				if (col % 2 == 0) {
					std::cout << (maze.verticalWalls[col / 2][row / 2] ? char(219) : ' ');
				}
				else {
					std::cout << ' ';
				}
			}
		}
		std::cout << std::endl;
	}
}

void printMazeSVG(const Maze & maze)
{
	std::ofstream f;
	f.open("maze.svg");

	f << "<?xml version='1.0' standalone='no'?>" << std::endl;
	f << "<svg width='210mm' height='297mm' version='1.1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 210 297'>" << std::endl;

	f << "<rect x='0' y='0' width='210' height='297' fill='white'/>" << std::endl;

	f << "<g stroke='black' stroke-linecap='round' stroke-width='0.5' >" << std::endl;

	const float w = 210;
	const float h = 297;
	const float margin = 20;

	const float maxStep = 10;
	const float fitStepW = (w - 2 * margin) / maze.width;
	const float fitStepH = (h - 2 * margin) / maze.height;
	const float fitStep = std::min(fitStepH, fitStepW);
	const float step = std::min(fitStep, maxStep);

	const float mazeWidth = step * maze.width;
	const float mazeHeight = step * maze.height;

	const float left = (w - mazeWidth) / 2;
	const float top = (h - mazeHeight) / 2;

	for (unsigned row = 0; row < maze.height + 1; ++row) {
		for (unsigned col = 0; col < maze.width + 1; ++col) {
			const bool closedTop = col < maze.width && maze.horizontalWalls[row][col];
			const bool closedLeft = row < maze.height && maze.verticalWalls[col][row];

			if (closedTop) {
				const float x = left + step * col;
				const float y = top + step * row;
				f << "<line x1='" << x << "' y1='" << y << "' x2='" << (x + step) << "' y2='" << y << "' />" << std::endl;
			}
			if (closedLeft) {
				const float x = left + step * col;
				const float y = top + step * row;
				f << "<line x1='" << x << "' y1='" << y << "' x2='" << x << "' y2='" << (y + step) << "' />" << std::endl;
			}

		}
	}

	f << "</g>" << std::endl;
	f << "</svg>" << std::endl;
}

int main()
{
	const unsigned width = 16;
	const unsigned height = 16;
	Maze m(width, height);

	placeEntryExit(m);
	generateArea(m, 0, 0, width, height);

	printMazeSVG(m);
}
