// maze.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <random>
#include <iostream>

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
		maze.horizontalWalls[maze.height][entry] = false;
	}
	else if (entrySelection - maze.width - maze.height - maze.width < maze.height) {
		// entry on left
		const unsigned entry = entrySelection - maze.width - maze.height - maze.width;
		maze.verticalWalls[0][entry] = false;
	}

}

void placeEntryExit(Maze & maze)
{
	const unsigned entryOptions = maze.width * 2 + maze.height * 2;
	std::uniform_int_distribution<> dis(0, entryOptions - 1);
	const unsigned entrySelection = dis(gen);

	placeXX(maze, entrySelection);

	std::uniform_int_distribution<> dis2(entrySelection + std::min(maze.width, maze.height), entrySelection + entryOptions - std::min(maze.width, maze.height));
	const unsigned exitSelection = dis2(gen) % entryOptions;

	placeXX(maze, exitSelection);
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
		std::uniform_int_distribution<> dis2(0, height - 1);
		const unsigned doorAt = dis2(gen);

		maze.verticalWalls[leftWall + splitAt + 1][topWall + doorAt] = false;

		generateArea(maze, leftWall, topWall, leftWall + splitAt + 1, bottomWall);
		generateArea(maze, leftWall + splitAt + 1, topWall, rightWall, bottomWall);
	}
	else { // horizontal split
		const unsigned splitAt = splitSelection - verticalSplitOptions;
		std::uniform_int_distribution<> dis2(0, width - 1);
		const unsigned doorAt = dis2(gen);

		maze.horizontalWalls[topWall + splitAt + 1][leftWall + doorAt] = false;

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

int main()
{
	const unsigned width = 20;
	const unsigned height = 20;
	Maze m(width, height);

	placeEntryExit(m);
	generateArea(m, 0, 0, width, height);

	printMaze2(m);
}

